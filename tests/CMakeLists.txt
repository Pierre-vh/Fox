# Add test sources to access variable ${tests_src}
add_subdirectory(src)

# Add executable & link libmoonshot & gtest
add_executable(MoonshotTests ${tests_src})
target_link_libraries(MoonshotTests libmoonshot gtest_main)

# Add include subdir : testutils
include_directories(src)

# add test resources path macro
target_compile_definitions(MoonshotTests PRIVATE TEST_RES_PATH="${CMAKE_CURRENT_SOURCE_DIR}")

# Add tests
add_test(NAME MoonshotTests COMMAND MoonshotTests)