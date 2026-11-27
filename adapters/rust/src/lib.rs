use std::ffi::c_char;
use std::fs;
use std::path::{Path, PathBuf};
use std::sync::OnceLock;
use std::time::Instant;

const BATCH_CAPACITY: usize = 32;
const MAX_CONTROLLERS: usize = 16;
const SOURCE_LINUX_EDAC: u32 = 2;
const SEVERITY_CORRECTED: u32 = 1;
const SEVERITY_UNCORRECTED: u32 = 2;
const EVENT_CORRECTED: u32 = 1;
const EVENT_UNCORRECTED: u32 = 2;
const FLAG_COUNTER_RESET: u32 = 1;
const FLAG_SYSTEM_WIDE: u32 = 2;
const FLAG_COUNTER_BASELINE: u32 = 4;
const STATUS_OK: i32 = 0;
const STATUS_UNSUPPORTED: i32 = -1;
const STATUS_INVALID_ARGUMENT: i32 = -2;
const STATUS_READ_ERROR: i32 = -3;
const STATUS_CAPACITY: i32 = -4;

#[repr(C)]
#[derive(Clone, Copy, Default)]
pub struct RasRecord {
    record_id: u64,
    timestamp_ns: u64,
    event_id: u32,
    level: u32,
    source: u32,
    severity: u32,
    count_delta: u64,
    controller_id: u32,
    flags: u32,
}

#[repr(C)]
pub struct RasBatch {
    records: [RasRecord; BATCH_CAPACITY],
    count: usize,
    dropped_records: u32,
    may_have_more: u8,
    reserved: [u8; 3],
}
impl Default for RasBatch {
    fn default() -> Self {
        Self {
            records: [RasRecord::default(); BATCH_CAPACITY],
            count: 0,
            dropped_records: 0,
            may_have_more: 0,
            reserved: [0; 3],
        }
    }
}

#[repr(C)]
pub struct RasCursor {
    last_record_id: u64,
    previous_ce: [u64; MAX_CONTROLLERS],
    previous_ue: [u64; MAX_CONTROLLERS],
    controller_ids: [u32; MAX_CONTROLLERS],
    controller_count: u32,
    initialized: u32,
    subscription_handle: usize,
    notification_handle: usize,
    provider_state: u32,
    reserved: u32,
}

#[derive(Clone, Copy)]
struct ControllerCounts {
    id: u32,
    corrected: u64,
    uncorrected: u64,
}

fn monotonic_now_ns() -> u64 {
    static START: OnceLock<Instant> = OnceLock::new();
    let elapsed = START.get_or_init(Instant::now).elapsed().as_nanos();
    elapsed.min(u64::MAX as u128) as u64
}
fn read_counter(path: &Path) -> Result<u64, ()> {
    let content = fs::read_to_string(path).map_err(|_| ())?;
    let trimmed = content.trim();
    if trimmed.is_empty() || !trimmed.bytes().all(|byte| byte.is_ascii_digit()) {
        return Err(());
    }
    trimmed.parse::<u64>().map_err(|_| ())
}
fn controller_id(path: &Path) -> Option<u32> {
    let name = path.file_name()?.to_str()?;
    name.strip_prefix("mc")?.parse::<u32>().ok()
}
fn scan_controllers(root: &Path) -> Result<Vec<ControllerCounts>, i32> {
    if !root.exists() { return Err(STATUS_UNSUPPORTED); }
    let entries = fs::read_dir(root).map_err(|_| STATUS_READ_ERROR)?;
    let mut paths: Vec<PathBuf> = entries.filter_map(Result::ok)
        .map(|entry| entry.path())
        .filter(|path| path.is_dir() && controller_id(path).is_some())
        .collect();
    paths.sort();
    let mut controllers = Vec::with_capacity(paths.len().min(MAX_CONTROLLERS));
    for path in paths.into_iter().take(MAX_CONTROLLERS) {
        let id = controller_id(&path).ok_or(STATUS_READ_ERROR)?;
        let corrected = read_counter(&path.join("ce_count")).map_err(|_| STATUS_READ_ERROR)?;
        let uncorrected = read_counter(&path.join("ue_count")).map_err(|_| STATUS_READ_ERROR)?;
        controllers.push(ControllerCounts { id, corrected, uncorrected });
    }
    Ok(controllers)
}
fn find_controller(cursor: &RasCursor, id: u32) -> Option<usize> {
    (0..(cursor.controller_count as usize).min(MAX_CONTROLLERS))
        .find(|index| cursor.controller_ids[*index] == id)
}
fn append_record(cursor: &mut RasCursor, batch: &mut RasBatch,
    controller: u32, event_id: u32, severity: u32, delta: u64, flags: u32
) -> Result<(), i32> {
    if delta == 0 { return Ok(()); }
    if batch.count >= BATCH_CAPACITY {
        batch.dropped_records = batch.dropped_records.saturating_add(1);
        batch.may_have_more = 1;
        return Err(STATUS_CAPACITY);
    }
    let next_id = cursor.last_record_id.checked_add(1).ok_or(STATUS_CAPACITY)?;
    cursor.last_record_id = next_id;
    batch.records[batch.count] = RasRecord {
        record_id: next_id,
        timestamp_ns: monotonic_now_ns(),
        event_id,
        level: 0,
        source: SOURCE_LINUX_EDAC,
        severity,
        count_delta: delta,
        controller_id: controller,
        flags: FLAG_SYSTEM_WIDE | flags,
    };
    batch.count += 1;
    Ok(())
}
fn poll_root(root: &Path, cursor: &mut RasCursor, batch: &mut RasBatch) -> i32 {
    let controllers = match scan_controllers(root) {
        Ok(value) => value,
        Err(status) => return status,
    };
    if cursor.controller_count as usize > MAX_CONTROLLERS {
        return STATUS_INVALID_ARGUMENT;
    }
    if cursor.initialized == 0 {
        cursor.controller_count = 0;
        for controller in controllers.iter().take(MAX_CONTROLLERS) {
            let slot = cursor.controller_count as usize;
            cursor.controller_ids[slot] = controller.id;
            cursor.previous_ce[slot] = controller.corrected;
            cursor.previous_ue[slot] = controller.uncorrected;
            cursor.controller_count += 1;
            if append_record(cursor, batch, controller.id, EVENT_CORRECTED,
                SEVERITY_CORRECTED, controller.corrected, FLAG_COUNTER_BASELINE).is_err() ||
               append_record(cursor, batch, controller.id, EVENT_UNCORRECTED,
                SEVERITY_UNCORRECTED, controller.uncorrected, FLAG_COUNTER_BASELINE).is_err() {
                return STATUS_CAPACITY;
            }
        }
        cursor.initialized = 1;
        return STATUS_OK;
    }
    for controller in controllers {
        let slot = match find_controller(cursor, controller.id) {
            Some(index) => index,
            None => {
                if cursor.controller_count as usize >= MAX_CONTROLLERS {
                    batch.dropped_records = batch.dropped_records.saturating_add(2);
                    batch.may_have_more = 1;
                    continue;
                }
                let index = cursor.controller_count as usize;
                cursor.controller_ids[index] = controller.id;
                cursor.previous_ce[index] = controller.corrected;
                cursor.previous_ue[index] = controller.uncorrected;
                cursor.controller_count += 1;
                if append_record(
                    cursor, batch, controller.id, EVENT_CORRECTED,
                    SEVERITY_CORRECTED, controller.corrected, FLAG_COUNTER_BASELINE,
                ).is_err() {
                    return STATUS_CAPACITY;
                }
                if append_record(
                    cursor, batch, controller.id, EVENT_UNCORRECTED,
                    SEVERITY_UNCORRECTED, controller.uncorrected, FLAG_COUNTER_BASELINE,
                ).is_err() {
                    return STATUS_CAPACITY;
                }
                continue;
            }
        };
        let previous_ce = cursor.previous_ce[slot];
        let previous_ue = cursor.previous_ue[slot];
        let corrected_reset = controller.corrected < previous_ce;
        let uncorrected_reset = controller.uncorrected < previous_ue;
        let corrected_delta = if corrected_reset { controller.corrected }
            else { controller.corrected - previous_ce };
        let uncorrected_delta = if uncorrected_reset { controller.uncorrected }
            else { controller.uncorrected - previous_ue };
        if append_record(cursor, batch, controller.id, EVENT_CORRECTED,
            SEVERITY_CORRECTED, corrected_delta,
            if corrected_reset { FLAG_COUNTER_RESET } else { 0 }).is_err() {
            return STATUS_CAPACITY;
        }
        if append_record(cursor, batch, controller.id, EVENT_UNCORRECTED,
            SEVERITY_UNCORRECTED, uncorrected_delta,
            if uncorrected_reset { FLAG_COUNTER_RESET } else { 0 }).is_err() {
            return STATUS_CAPACITY;
        }
        cursor.previous_ce[slot] = controller.corrected;
        cursor.previous_ue[slot] = controller.uncorrected;
    }
    STATUS_OK
}

#[no_mangle]
pub extern "C" fn pl_ras_provider_name() -> *const c_char {
    b"Linux EDAC sysfs counters (read-only, monotonic deltas)\0".as_ptr() as *const c_char
}
#[no_mangle]
pub unsafe extern "C" fn pl_ras_provider_poll(
    cursor: *mut RasCursor, batch: *mut RasBatch,
) -> i32 {
    if cursor.is_null() || batch.is_null() { return STATUS_INVALID_ARGUMENT; }
    let cursor = &mut *cursor;
    let batch = &mut *batch;
    *batch = RasBatch::default();
    poll_root(Path::new("/sys/devices/system/edac/mc"), cursor, batch)
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::time::{SystemTime, UNIX_EPOCH};
    fn temporary_root() -> PathBuf {
        let unique = SystemTime::now().duration_since(UNIX_EPOCH).unwrap().as_nanos();
        let mc = std::env::temp_dir().join(format!("ptfr-edac-{unique}")).join("mc0");
        fs::create_dir_all(&mc).unwrap();
        fs::write(mc.join("ce_count"), "0\n").unwrap();
        fs::write(mc.join("ue_count"), "0\n").unwrap();
        mc.parent().unwrap().to_path_buf()
    }
    #[test]
    fn first_poll_records_existing_cumulative_counters_as_baseline() {
        let root = temporary_root();
        let mc = root.join("mc0");
        fs::write(mc.join("ce_count"), "9\n").unwrap();
        fs::write(mc.join("ue_count"), "2\n").unwrap();
        let mut cursor = unsafe { std::mem::zeroed::<RasCursor>() };
        let mut batch = RasBatch::default();
        assert_eq!(poll_root(&root, &mut cursor, &mut batch), STATUS_OK);
        assert_eq!(batch.count, 2);
        assert_ne!(batch.records[0].flags & FLAG_COUNTER_BASELINE, 0);
        assert_eq!(batch.records[0].count_delta, 9);
        fs::remove_dir_all(root).unwrap();
    }
    #[test]
    fn subsequent_counter_deltas_become_typed_records() {
        let root = temporary_root();
        let mc = root.join("mc0");
        let mut cursor = unsafe { std::mem::zeroed::<RasCursor>() };
        let mut batch = RasBatch::default();
        assert_eq!(poll_root(&root, &mut cursor, &mut batch), STATUS_OK);
        fs::write(mc.join("ce_count"), "5\n").unwrap();
        fs::write(mc.join("ue_count"), "1\n").unwrap();
        assert_eq!(poll_root(&root, &mut cursor, &mut batch), STATUS_OK);
        assert_eq!(batch.count, 2);
        assert_eq!(batch.records[0].source, SOURCE_LINUX_EDAC);
        assert_eq!(batch.records[0].severity, SEVERITY_CORRECTED);
        assert_eq!(batch.records[0].count_delta, 5);
        assert_eq!(batch.records[1].severity, SEVERITY_UNCORRECTED);
        assert_eq!(batch.records[1].count_delta, 1);
        fs::remove_dir_all(root).unwrap();
    }
    #[test]
    fn counter_resets_are_marked_not_misread_as_negative_deltas() {
        let root = temporary_root();
        let mc = root.join("mc0");
        let mut cursor = unsafe { std::mem::zeroed::<RasCursor>() };
        let mut batch = RasBatch::default();
        assert_eq!(poll_root(&root, &mut cursor, &mut batch), STATUS_OK);
        fs::write(mc.join("ce_count"), "7\n").unwrap();
        assert_eq!(poll_root(&root, &mut cursor, &mut batch), STATUS_OK);
        fs::write(mc.join("ce_count"), "1\n").unwrap();
        assert_eq!(poll_root(&root, &mut cursor, &mut batch), STATUS_OK);
        assert_eq!(batch.count, 1);
        assert_eq!(batch.records[0].count_delta, 1);
        assert_ne!(batch.records[0].flags & FLAG_COUNTER_RESET, 0);
        fs::remove_dir_all(root).unwrap();
    }
    #[test]
    fn unsupported_root_is_reported_explicitly() {
        let root = std::env::temp_dir().join("ptfr-edac-nonexistent-root");
        let mut cursor = unsafe { std::mem::zeroed::<RasCursor>() };
        let mut batch = RasBatch::default();
        assert_eq!(poll_root(&root, &mut cursor, &mut batch), STATUS_UNSUPPORTED);
    }
}


#[no_mangle]
pub unsafe extern "C" fn pl_ras_provider_close(_cursor: *mut RasCursor) {
    /* Linux sysfs has no subscription handle to release. */
}
