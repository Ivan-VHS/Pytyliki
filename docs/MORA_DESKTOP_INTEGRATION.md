# Mora desktop integration — Pytyliki

Pytyliki joins Mora as the processor-systems workstation. Mora supplies a shared desktop catalogue and startup theme; the C11 core, portable CLI tools and Windows native GUI remain owned by this repository.

## Workspace contract

- Manifest: mora-app.json, version 1.
- Native GUI: Windows Win32 target pytyliki-gui.
- Theme hand-off: MORA_THEME_MODE=dark or light, read before the native window and its brushes are created.
- Window model: separate OS-managed native window.
- Portable core and command-line targets remain independent of the desktop shell.

Mora only enables the launch action when it finds a built Windows GUI target. On Linux and macOS it may still list the project and its portable tools, but it does not pretend that the Windows GUI is available.

The shared application schema and local launcher are maintained in [DAK / Mora](https://github.com/Ivan-VHS/DAK/blob/main/docs/MORA_DESKTOP.md).

## Shared appearance

The inherited startup preference selects the existing blue/red light or dark palette before the UI brushes are created. It does not add a new theme framework or couple the core library to UI code. An in-app theme selector, where present, can still change the current session.

## Product boundaries

- CPU inventory and system inspection remain local and read-only.
- Measurements stay bounded and report host-dependent results.
- Pytyliki does not modify scheduler policy, affinity, clocks, voltage or firmware.
- Embedded MCU/MMIO material remains reference or simulation data.
- QCS remains syntax analysis, not arbitrary source execution.
- Mora does not elevate privileges or execute CLI text supplied by a browser.

## Verification

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
    cmake --build build --config Release --parallel
    ctest --test-dir build -C Release --output-on-failure

The native GUI is built only on Windows. The portable C11 core and CLI keep their existing platform-specific build rules.
