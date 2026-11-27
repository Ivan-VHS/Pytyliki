#![deny(unsafe_op_in_unsafe_fn)]

use std::ffi::{CStr, c_char, c_int, c_void};
use std::fs;
use std::path::{Path, PathBuf};
use std::time::{SystemTime, UNIX_EPOCH};

const SOURCE_CAPACITY: usize = 96;
const HASH_CAPACITY: usize = 65;
const MAX_EVENTS: usize = 256;
const MAX_DEPTH: usize = 5;
const MAX_VISITED: usize = 2048;
const PROVIDER_LINUX_EDAC: u32 = 2;
const SEVERITY_WARNING: u32 = 2;
const SEVERITY_ERROR: u32 = 3;
const UNKNOWN_CPU: u32 = u32::MAX;

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct PlTrPlatformEvent {
    timestamp_epoch_ns: u64,
    source_record_id: u64,
    counter_total: u64,
    provider: u32,
    event_code: u32,
    cpu_id: u32,
    severity: u32,
    kind: u32,
    source_identity: [c_char; SOURCE_CAPACITY],
    payload_sha256: [c_char; HASH_CAPACITY],
}

type EventSink = Option<unsafe extern "C" fn(*const PlTrPlatformEvent, *mut c_void) -> c_int>;

fn timestamp_epoch_ns() -> u64 {
    let duration = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap_or_default();
    duration.as_nanos().min(u64::MAX as u128) as u64
}

fn counter_kind(name: &str) -> Option<(u32, u32, u32)> {
    match name {
        "ce_count" | "dimm_ce_count" | "edac_ce_count" =>
            Some((1, 4, SEVERITY_WARNING)),
        "ue_count" | "dimm_ue_count" | "edac_ue_count" =>
            Some((2, 4, SEVERITY_ERROR)),
        _ => None,
    }
}

fn fixed_c_string<const N: usize>(value: &str) -> Option<[c_char; N]> {
    if value.is_empty() || value.len() >= N || !value.is_ascii() {
        return None;
    }
    let mut output = [0 as c_char; N];
    for (index, byte) in value.bytes().enumerate() {
        if !(byte.is_ascii_alphanumeric() || b"_./-".contains(&byte)) {
            return None;
        }
        output[index] = byte as c_char;
    }
    Some(output)
}

fn make_event(root: &Path, path: &Path, counter_name: &str, total: u64) -> Option<PlTrPlatformEvent> {
    let relative = path.strip_prefix(root).ok()?.to_str()?;
    let source_identity = fixed_c_string::<SOURCE_CAPACITY>(relative)?;
    let (event_code, kind, severity) = counter_kind(counter_name)?;
    Some(PlTrPlatformEvent {
        timestamp_epoch_ns: timestamp_epoch_ns(),
        source_record_id: 0,
        counter_total: total,
        provider: PROVIDER_LINUX_EDAC,
        event_code,
        cpu_id: UNKNOWN_CPU,
        severity,
        kind,
        source_identity,
        payload_sha256: [0; HASH_CAPACITY],
    })
}

fn scan_tree(root: &Path) -> Result<Vec<PlTrPlatformEvent>, i32> {
    if !root.is_dir() {
        return Err(-2);
    }
    let mut events = Vec::new();
    let mut pending: Vec<(PathBuf, usize)> = Vec::new();
    let mut visited = 0usize;

    for child in ["mc", "devices"] {
        let path = root.join(child);
        if path.is_dir() {
            pending.push((path, 0));
        }
    }
    if pending.is_empty() {
        return Err(-3);
    }

    while let Some((directory, depth)) = pending.pop() {
        visited += 1;
        if visited > MAX_VISITED || depth > MAX_DEPTH {
            return Err(-3);
        }
        let entries = fs::read_dir(&directory).map_err(|_| -4)?;
        let mut directory_entries = 0usize;
        for entry in entries {
            directory_entries += 1;
            if directory_entries > MAX_VISITED {
                return Err(-3);
            }
            let entry = entry.map_err(|_| -4)?;
            let path = entry.path();
            let file_type = entry.file_type().map_err(|_| -4)?;
            if file_type.is_dir() {
                if depth < MAX_DEPTH {
                    pending.push((path, depth + 1));
                }
                continue;
            }
            if !file_type.is_file() {
                continue;
            }
            let name = entry.file_name();
            let Some(counter_name) = name.to_str() else { continue };
            if counter_kind(counter_name).is_none() {
                continue;
            }
            let raw = fs::read_to_string(&path).map_err(|_| -4)?;
            let total = raw.trim().parse::<u64>().map_err(|_| -5)?;
            let event = make_event(root, &path, counter_name, total).ok_or(-6)?;
            if events.len() == MAX_EVENTS {
                return Err(-7);
            }
            events.push(event);
        }
    }

    events.sort_by(|left, right| {
        let left_name = unsafe { CStr::from_ptr(left.source_identity.as_ptr()) };
        let right_name = unsafe { CStr::from_ptr(right.source_identity.as_ptr()) };
        left_name.to_bytes().cmp(right_name.to_bytes())
    });
    Ok(events)
}

/// Read only. The caller chooses the sysfs root and receives normalized cumulative counters.
/// Return: number of events, or a negative error code. A callback error aborts the scan.
#[no_mangle]
pub unsafe extern "C" fn pytyliki_edac_scan(
    root: *const c_char,
    sink: EventSink,
    context: *mut c_void,
) -> c_int {
    let outcome = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        if root.is_null() || sink.is_none() {
            return -1;
        }
        let root_text = unsafe { CStr::from_ptr(root) };
        let Ok(root_text) = root_text.to_str() else { return -1 };
        let events = match scan_tree(Path::new(root_text)) {
            Ok(events) => events,
            Err(code) => return code,
        };
        let callback = sink.expect("checked above");
        for event in &events {
            let result = unsafe { callback(event as *const _, context) };
            if result != 0 {
                return -8;
            }
        }
        events.len() as c_int
    }));
    outcome.unwrap_or(-9)
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::fs;
    use std::sync::atomic::{AtomicU64, Ordering};

    static NEXT: AtomicU64 = AtomicU64::new(1);

    fn fixture_root() -> PathBuf {
        let path = std::env::temp_dir().join(format!(
            "pytyliki-edac-{}-{}",
            std::process::id(),
            NEXT.fetch_add(1, Ordering::Relaxed)
        ));
        fs::create_dir_all(path.join("mc/mc0/dimm0")).unwrap();
        fs::write(path.join("mc/mc0/ce_count"), "7\n").unwrap();
        fs::write(path.join("mc/mc0/ue_count"), "0\n").unwrap();
        fs::write(path.join("mc/mc0/dimm0/dimm_ce_count"), "3\n").unwrap();
        path
    }

    #[test]
    fn enumerates_corrected_and_uncorrected_counters_read_only() {
        let root = fixture_root();
        let events = scan_tree(&root).unwrap();
        assert_eq!(events.len(), 3);
        assert!(events.iter().any(|event| event.event_code == 1 && event.kind == 4 && event.counter_total == 7));
        assert!(events.iter().any(|event| event.event_code == 2 && event.kind == 4 && event.counter_total == 0));
        assert!(events.iter().any(|event| event.event_code == 1 && event.kind == 4 && event.counter_total == 3));
        assert!(events.iter().all(|event| event.cpu_id == UNKNOWN_CPU));
        fs::remove_dir_all(root).unwrap();
    }

    #[test]
    fn rejects_missing_root_and_non_numeric_counter() {
        let missing = std::env::temp_dir().join("pytyliki-edac-does-not-exist");
        assert_eq!(scan_tree(&missing).unwrap_err(), -2);
        let root = fixture_root();
        fs::write(root.join("mc/mc0/ce_count"), "bad").unwrap();
        assert_eq!(scan_tree(&root).unwrap_err(), -5);
        fs::remove_dir_all(root).unwrap();
    }

    #[test]
    fn bounds_the_source_identity_and_counter_inventory() {
        assert!(fixed_c_string::<SOURCE_CAPACITY>("mc/mc0/ce_count").is_some());
        assert!(fixed_c_string::<SOURCE_CAPACITY>(&"x".repeat(SOURCE_CAPACITY)).is_none());
        assert!(counter_kind("unknown").is_none());
    }
}
