# Platform validation policy

| Gate | Evidence |
|---|---|
| Configure/build | CMake + C11 compiler |
| Tests | CTest on supported OS |
| Portability | Windows/Linux CI where available |
| Native safety | sanitizers and bounds checks |
| Limits | unsupported platform/capability remains explicit |

A passing build does not certify all hardware or drivers.
