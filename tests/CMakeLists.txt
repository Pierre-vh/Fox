# We need thread support
find_package(Threads REQUIRED)

configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(
    COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download
    )

if (result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()

execute_process(
    COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download
    )

if (result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL
                 )

# Add test sources to access variable ${tests_src}
add_subdirectory(src)

# Add /Profile flag under MSVC to Enable profiling 
if(MSVC)
	SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} /Profile" )
endif()

# Add executable & link libmoonshot & gtest
add_executable(MoonshotTests ${tests_src})
target_link_libraries(MoonshotTests libmoonshot gtest_main)

# Add include subdir : testutils
include_directories(src)

# add test resources path macro
target_compile_definitions(MoonshotTests PRIVATE TEST_RES_PATH="${CMAKE_CURRENT_SOURCE_DIR}")

# Add tests
add_test(NAME MoonshotTests COMMAND MoonshotTests)