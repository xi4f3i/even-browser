find_package(GTest CONFIG REQUIRED)

set(TEST_SOURCES
    html/tokenizer_tests.cpp
)

add_executable(even-browser-tests ${TEST_SOURCES})

target_link_libraries(even-browser-tests PRIVATE 
    GTest::gtest_main
    even-core
)

include(GoogleTest)
gtest_discover_tests(even-browser-tests)