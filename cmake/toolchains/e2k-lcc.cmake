# Optional MCST Elbrus E2K cross/native build profile.
# Supply the licensed MCST LCC compiler and matching target sysroot.
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR e2k)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

if(NOT DEFINED CMAKE_C_COMPILER)
  if(DEFINED ENV{MCST_LCC} AND NOT "$ENV{MCST_LCC}" STREQUAL "")
    set(CMAKE_C_COMPILER "$ENV{MCST_LCC}" CACHE FILEPATH "MCST LCC C compiler")
  else()
    find_program(CMAKE_C_COMPILER NAMES lcc)
    if(NOT CMAKE_C_COMPILER)
      message(FATAL_ERROR
        "MCST LCC not found. Install the matching MCST SDK and set MCST_LCC "
        "or pass -DCMAKE_C_COMPILER=/path/to/lcc. Supply a matching sysroot "
        "when cross-compiling.")
    endif()
  endif()
endif()

# No generation-specific -march/-m* flags are injected.
