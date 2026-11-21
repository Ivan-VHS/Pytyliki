# Pytyliki Win32 UI motion contract — 2026

The Win32 workstation adopts the shared Motion Lab 2026 accessibility profile without changing C11 processor, FFI, benchmark, PCM or DSP execution paths.

## Modes and timing

- **Normal** uses the restrained 140 ms native startup blend.
- **Expressive** uses a 240 ms native vertical glide (approximately 1.35x the standard transition token); Normal keeps the 140 ms startup blend.
- **Reduced Motion** disables the startup transition and takes precedence over Expressive.
- **Eco / Low-CPU Visuals** disables the startup transition and nonessential visual work.

The selector and Eco preference are persisted under `Software\\Ivan-VHS\\Pytyliki\\UI`. The legacy `ReduceMotion` registry value remains readable and writable for backwards compatibility. `UI_REDUCED_MOTION` and the Windows client-area animation preference override application settings.

## Visual and interaction translation

Native owner-drawn buttons retain restrained hover/pressed feedback, visible focus rectangles and unchanged hit targets. Blue `#0039A6` / navy `#0B1F3A` communicate navigation and calm state; red `#D52B1E` / `#B51A10` is reserved for critical, mutation and safety feedback.

The browser-only mesh blur, Liquid Glass filtering, pointer magnets, 3D portal tilt and scroll parallax are not injected into the native processing workstation. Their intent is translated into clear native surfaces, controlled state transitions and high-contrast focus feedback.

All visual transitions remain separate from processor workers, FFI, benchmarks and audio/DSP operations. BPM sync is unavailable because this workstation has no project transport or master-tempo source.


## Anatidae / Pond Motion translation

The shared profile maps the duck's short paddle response to one bounded Win32 startup glide. It does not add a perpetual parallax loop, particles, blur or animated backgrounds to the processor workbench. The target paddle response is `cubic-bezier(0.22, 1.18, 0.36, 1)`; the symmetric water curve `cubic-bezier(0.37, 0, 0.63, 1)` is reserved for optional showcase artwork. Reduce Motion, Eco and the Windows client-area animation preference continue to suppress the startup transition entirely.

See the [shared Anatidae motion constitution](ANATIDAE_MOTION_CONSTITUTION.md).

## Shared 2026 contract

The shared semantic and accessibility contract is documented in [UI_UX_2026_CONVERGENCE.md](UI_UX_2026_CONVERGENCE.md). Win32-specific motion remains deliberately restrained; no animation is added to precision or processor paths.
