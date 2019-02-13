# include GoogleTest.cmake. This will create the "googletest"
# target and define the googletest_includes variable.
include("${PROJECT_SOURCE_DIR}/thirdparty/GoogleTest.cmake")

# Add test sources to access variable ${tests_src}
add_subdirectory(src)

# Add executable & link libmoonshot & gtest
add_executable(fox_unittests ${unittest_src})
target_link_libraries(fox_unittests libfox googletest)

# add src as an include directory, so we can access headers
# such as TestUtils
include_directories(src)

# add GoogleTest include directory
include_directories(${googletest_includes})

# add test resources path macro
target_compile_definitions(fox_unittests 
  PRIVATE TEST_RES_PATH="${CMAKE_CURRENT_SOURCE_DIR}")

# Add tests
add_test(NAME fox_unittests COMMAND fox_unittests)