cmake_minimum_required(VERSION 3.0)
project(Moonshot)

# We'll need python
include(FindPythonInterp)

#set minimal C++ standard 
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#include add_source macro/fn definitions
include(cmake/AddSource.cmake)

#include googletest related stuff 
include(cmake/GoogleTest.cmake)

# set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# set include directories
include_directories(includes)   # Moonshot lib includes
include_directories(thirdparty) # Third Party libraries includes

# add main subdirs
add_subdirectory(lib)
add_subdirectory(tools/CommandLine)

# add test subdir
add_subdirectory(unittests)

# MSVC flags
if(MSVC)
  # Warnings level 4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
  # Disable RTTI
  if(CMAKE_CXX_FLAGS MATCHES "/GR")
      string(REGEX REPLACE "/GR" "/GR-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR-")
  endif()
  # Set Warnings are errors & disable extensions
  add_compile_options(/WX)
  #add_compile_options(/Za)
  #Silence problematic warnings: 
  add_compile_options(/wd4706) # "assignement within conditional expression" is rather pedantic.
  add_compile_options(/wd4324) # "structure was padded due to alignement specifier" no sh*t sherlock.
endif()

# add lib target
add_library(libmoonshot STATIC ${moonshot_src})

# add executable target & link it with libmoonshot
add_executable(Moonshot ${cli_src})
target_link_libraries(Moonshot libmoonshot)

# add llvm lit test suite 
add_custom_target(MoonshotLit 
  COMMAND lit ${PROJECT_SOURCE_DIR}/tests -s --param fox_bin=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  DEPENDS Moonshot 
)

# add test resources path macro
target_compile_definitions(libmoonshot PRIVATE TEST_RES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/tests/res/")