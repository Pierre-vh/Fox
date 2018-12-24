cmake_minimum_required(VERSION 3.0)
project(Moonshot)

include(FindPythonInterp)

# set minimal C++ standard 
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# include most used cmake utilities
include(cmake/AddSource.cmake)
include(cmake/MSVCUtils.cmake)

# include googletest related stuff 
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
  # warnings level 4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
  # disable RTTI
  if(CMAKE_CXX_FLAGS MATCHES "/GR")
      string(REGEX REPLACE "/GR" "/GR-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR-")
  endif()
  # set compiler flags
  add_compile_options(/WX)
  add_compile_options(/Za)
  add_compile_options(/permissive-)
  # silence problematic warnings 
    # C4706 "assignement within conditional expression"
    #   Reason: This construct is used a lot in the code
    add_compile_options(/wd4706) 
  
    # C4324 "structure was padded due to alignement specifier"
    # C4458 "declaration of x hides class member"
    #   Reason: That's usually the intended behaviour
    add_compile_options(/wd4324)
    add_compile_options(/wd4458)
  
    # C4291 "no matching operator delete found; memory will not be freed if initialization fails"
    #   Reason: This error happens for the operator new overloads of the AST, which allocate memory
    #           in the ASTContext, and we can't free a single block of memory without freeing everything.
    add_compile_options(/wd4291)

    # C4244 "conversion from 'a' to 'b', possible loss of data"
    #   Reason: This warning only exist on MSVC. 
    add_compile_options(/wd4244)
  
  # don't deprecate theses functions
  add_compile_options(-D_SCL_SECURE_NO_WARNINGS)
endif()

# add lib target
add_library(libfox STATIC ${libfox_src})

# add llvm lit test suite 
add_custom_target(fox_tests
  COMMAND lit ${PROJECT_SOURCE_DIR}/tests -s -v 
  --param fox_bin_dir=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  DEPENDS fox
)

# add test resources path macro
target_compile_definitions(libfox PRIVATE 
  TEST_RES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/tests/res/")