# Pytyliki Win32 UI motion contract

The processor workstation follows the AETHER-ANAS white/blue/red motion policy. Native C11 processing, FFI, benchmarks and PCM/DSP paths are not animation hosts and remain untouched.

## Accessibility settings

The native UI exposes **Motion Settings**:
- **Reduce Motion** disables the startup window transition.
- **Eco / Low-CPU Visuals** suppresses nonessential visual effects; the current Win32 surface has no continuously animated graphs or particle scenes to throttle.
- **Sync to Transport / BPM** is displayed as unavailable because this workstation has no project transport or master-BPM source.

Preferences are stored under the current user's `Software\Ivan-VHS\Pytyliki\UI` registry key. The Windows client-area animation preference and `UI_REDUCED_MOTION` environment override are also respected.

## Visual rules

Blue `#0039A6` and navy `#0B1F3A` represent navigation and calm state. Red `#D52B1E` / `#B51A10` is reserved for critical/generation/safety feedback. Large-area flashing, parallax jolts and decorative rainbow effects are forbidden. Animation must never enter audio/DSP or processor worker code.
