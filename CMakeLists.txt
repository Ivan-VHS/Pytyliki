cmake_minimum_required(VERSION 3.16)
project(PytylikiCPULab VERSION 0.5.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(pytyliki_core
    src/core.c
    src/policy.c
    src/pipeline.c
    src/clinical_rules.c
)
target_include_directories(pytyliki_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(MSVC)
    target_compile_options(pytyliki_core PRIVATE /W4)
else()
    target_compile_options(pytyliki_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(WIN32)
    add_executable(pytyliki-gui WIN32 src/win_gui.c)
    target_link_libraries(pytyliki-gui PRIVATE pytyliki_core user32 gdi32)
endif()

add_executable(pytyliki src/cli.c)
target_link_libraries(pytyliki PRIVATE pytyliki_core)

include(CTest)
if(BUILD_TESTING)
    add_executable(pytyliki-tests tests/test_core.c)
    target_link_libraries(pytyliki-tests PRIVATE pytyliki_core)
    add_test(NAME core-tests COMMAND pytyliki-tests)
endif()
