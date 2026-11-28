target_sources(pytyliki_core PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../src/native_intelligence.c
)
if(WIN32)
    set_source_files_properties(
        ${CMAKE_CURRENT_LIST_DIR}/../src/native_intelligence.c
        PROPERTIES COMPILE_DEFINITIONS "_WIN32_WINNT=0x0A00;WINVER=0x0A00"
    )
endif()
if(BUILD_TESTING)
    add_executable(pytyliki-native-intelligence-tests
        ${CMAKE_CURRENT_LIST_DIR}/../tests/test_native_intelligence.c
    )
    target_link_libraries(pytyliki-native-intelligence-tests PRIVATE pytyliki_core)
    add_test(NAME native-intelligence-collector-ppf-resources
        COMMAND pytyliki-native-intelligence-tests
    )
endif()
install(FILES ${CMAKE_CURRENT_LIST_DIR}/../src/native_intelligence.h
    DESTINATION include/pytyliki
)

# Release builds define NDEBUG globally; contract tests must still execute assertions.
set(_pytyliki_assert_test_targets
    pytyliki-native-intelligence-tests
    pytyliki-interop-tests
    pytyliki-low-level-contract-tests
)
if(MSVC)
    foreach(_target IN LISTS _pytyliki_assert_test_targets)
        if(TARGET ${_target})
            target_compile_options(${_target} PRIVATE /UNDEBUG)
        endif()
    endforeach()
else()
    foreach(_target IN LISTS _pytyliki_assert_test_targets)
        if(TARGET ${_target})
            target_compile_options(${_target} PRIVATE -UNDEBUG)
        endif()
    endforeach()
endif()
