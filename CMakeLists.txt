cmake_minimum_required(VERSION 3.0)
project(Moonshot)

#set minimal C++ standard 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#include add_source macro/fn definitions
include(cmake/AddSource.cmake)

# set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Moonshot/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Moonshot/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Moonshot/bin)

# set include directories
include_directories(lib)        # Moonshot lib source
include_directories(thirdparty) # Third Party libraries includes

# add main subdirs
add_subdirectory(lib)
add_subdirectory(tools)

# add test subdir
add_subdirectory(tests)

# add lib target
add_library(libmoonshot STATIC ${lib_src})

# add executable target & link it with libmoonshot
add_executable(Moonshot ${tools_src})
target_link_libraries(Moonshot libmoonshot)

# add test resources path macro
target_compile_definitions(libmoonshot PRIVATE TEST_RES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/tests/res/")