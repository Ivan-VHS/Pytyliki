# Pytyliki Win32 UI motion contract — 2026

The Win32 workstation adopts the shared Motion Design 2026 accessibility profile without changing C11 processor, FFI, benchmark, PCM or DSP execution paths.

## Modes

- **Normal** uses the existing restrained 140 ms startup fade.
- **Expressive** uses a 190 ms native vertical window glide; Normal keeps a 140 ms blend fade. No animated geometry is added to performance-critical controls.
- **Reduced Motion** disables the startup transition and takes precedence over Expressive.
- **Eco / Low-CPU Visuals** disables the startup transition and nonessential visual work.

The motion selector and Eco preference are persisted under `Software\Ivan-VHS\Pytyliki\UI`. The legacy `ReduceMotion` registry value is still read and written for backwards compatibility. `UI_REDUCED_MOTION` and the Windows client-area animation preference override application settings.

## Visual and interaction rules

Native owner-drawn buttons already provide restrained hover/pressed feedback. Focus rectangles and hit targets remain unchanged. Blue `#0039A6` / navy `#0B1F3A` communicate navigation and calm state; red `#D52B1E` / `#B51A10` is reserved for critical, mutation and safety feedback. CSS backdrop blur, animated rainbow mesh, pointer parallax and large-area flashing are not fabricated in Win32.

All visual transitions remain separate from processor workers, FFI, benchmarks and audio/DSP operations. BPM sync is unavailable because this workstation has no project transport or master-tempo source.

## Anatidae / Pond Motion translation

The shared profile maps the duck's short paddle response to one bounded Win32 startup glide. It does not add a perpetual parallax loop, particles, blur or animated backgrounds to the processor workbench. The target paddle response is `cubic-bezier(0.22, 1.18, 0.36, 1)`; the symmetric water curve `cubic-bezier(0.37, 0, 0.63, 1)` is reserved for optional showcase artwork. Reduce Motion, Eco and the Windows client-area animation preference continue to suppress the startup transition entirely.
