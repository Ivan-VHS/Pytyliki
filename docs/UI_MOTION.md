# Pytyliki Win32 UI motion contract

The processor workstation follows the AETHER-ANAS white/blue/red motion policy. Native C11 processing, FFI, benchmarks and PCM/DSP paths are not animation hosts and remain untouched. The 2026 Motion Lab reference is adapted to native Win32 feedback rather than importing browser CSS, blur filters or cursor-driven 3D motion.

## Accessibility settings

The native UI exposes **Motion Settings**:

- **Normal** uses the short native startup transition.
- **Expressive Motion** selects a slower native startup blend for a more noticeable but bounded presentation.
- **Reduce Motion** disables the startup window transition.
- **Eco / Low-CPU Visuals** suppresses nonessential visual effects.
- **Sync to Transport / BPM** is displayed as unavailable because this workstation has no project transport or master-BPM source.

Preferences are stored under the current user's `Software\\Ivan-VHS\\Pytyliki\\UI` registry key. The Windows client-area animation preference and `UI_REDUCED_MOTION` environment override are also respected. Existing settings remain valid; Expressive Motion defaults to off.

## Visual rules

Blue `#0039A6` and navy `#0B1F3A` represent navigation and calm state. Red `#D52B1E` / `#B51A10` is reserved for critical/generation/safety feedback. Glass-like surfaces are represented by clear native surfaces and restrained border/highlight states. Large-area flashing, pointer magnets, parallax jolts, animated mesh backgrounds and decorative rainbow effects are forbidden. Animation must never enter audio/DSP, processor worker, benchmark or FFI code.
