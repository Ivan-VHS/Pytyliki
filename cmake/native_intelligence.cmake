target_sources(pytyliki_core PRIVATE
    \${CMAKE_CURRENT_LIST_DIR}/../src/native_intelligence.c
)
if(BUILD_TESTING)
    add_executable(pytyliki-native-intelligence-tests
        \${CMAKE_CURRENT_LIST_DIR}/../tests/test_native_intelligence.c
    )
    target_link_libraries(pytyliki-native-intelligence-tests PRIVATE pytyliki_core)
    add_test(NAME native-intelligence-collector-ppf-resources
        COMMAND pytyliki-native-intelligence-tests
    )
endif()
install(FILES \${CMAKE_CURRENT_LIST_DIR}/../src/native_intelligence.h
    DESTINATION include/pytyliki
)
