# Add test sources to access variable ${tests_src}
add_subdirectory(src)

# Add executable & link libmoonshot & gtest
add_executable(fox_unittests ${fox_unittests_src})
target_link_libraries(fox_unittests libfox gtest_main)

# Add include subdir : testutils
include_directories(src)

# add test resources path macro
target_compile_definitions(fox_unittests 
  PRIVATE TEST_RES_PATH="${CMAKE_CURRENT_SOURCE_DIR}")

# Add tests
add_test(NAME fox_unittests COMMAND fox_unittests)