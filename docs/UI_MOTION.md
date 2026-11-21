# Pytyliki Win32 UI motion contract — 2026

The Win32 workstation adopts the shared Motion Lab 2026 accessibility profile without changing C11 processor, FFI, benchmark, PCM or DSP execution paths.

## Modes and timing

- **Normal** uses the restrained 140 ms native startup blend.
- **Expressive** uses a more pronounced 240 ms blend (approximately 1.35x the standard transition token).
- **Reduced Motion** disables the startup transition and takes precedence over Expressive.
- **Eco / Low-CPU Visuals** disables the startup transition and nonessential visual work.

The selector and Eco preference are persisted under `Software\\Ivan-VHS\\Pytyliki\\UI`. The legacy `ReduceMotion` registry value remains readable and writable for backwards compatibility. `UI_REDUCED_MOTION` and the Windows client-area animation preference override application settings.

## Visual and interaction translation

Native owner-drawn buttons retain restrained hover/pressed feedback, visible focus rectangles and unchanged hit targets. Blue `#0039A6` / navy `#0B1F3A` communicate navigation and calm state; red `#D52B1E` / `#B51A10` is reserved for critical, mutation and safety feedback.

The browser-only mesh blur, Liquid Glass filtering, pointer magnets, 3D portal tilt and scroll parallax are not injected into the native processing workstation. Their intent is translated into clear native surfaces, controlled state transitions and high-contrast focus feedback.

All visual transitions remain separate from processor workers, FFI, benchmarks and audio/DSP operations. BPM sync is unavailable because this workstation has no project transport or master-tempo source.
