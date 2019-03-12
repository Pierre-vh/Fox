message("CMake version is '${CMAKE_VERSION}'")
cmake_minimum_required(VERSION 3.0)

# set project name
project(Fox)

# set minimal C++ standard: we need C++14 at a minimum.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# we'll want to export the compile commands as this is pretty 
# useful for some IDE/text editors out there.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# include the commonly used macros/helpers.
include(cmake/AddSource.cmake) # defines the add_source macro

# include third-party libraries script
include(thirdparty/ThirdParty.cmake)

# set the preferred output directories:
#   Archives go in the /lib folder
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
#   Libraries go in the /lib folder
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
#   Executables go in the /bin folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# determine our environment/compiler and put the results in variables.
# these variables will be used to determine which compile flags to use.

# check if 64 bits or not
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(IS_64BITS TRUE)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(IS_64BITS FALSE)
endif()

# fetch CXX environment variable
if(DEFINED ENV{CXX})
  set(CXX $ENV{CXX})
else()
  set(CXX "")
endif()

# check if clang-cl (on windows)
if(CXX STREQUAL "clang-cl" OR CXX MATCHES "clang-cl.exe")
  set(IS_CLANGCL TRUE)
else()
  set(IS_CLANGCL FALSE)
endif()

# check if clang (can be clang-cl!)
if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
  set(IS_CLANG TRUE)
else()
  set(IS_CLANG FALSE)
endif()

# check if MSVC
if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
  set(IS_MSVC TRUE)
else()
  set(IS_MSVC FALSE)
endif()

# check if MSVC or clang-cl
if(IS_MSVC OR IS_CLANGCL)
  set(IS_MSVC_OR_CLANGCL TRUE)
else()
  set(IS_MSVC_OR_CLANGCL FALSE)
endif()

# print the variables we just gathered.
message("")
message("Compiler ID is '${CMAKE_CXX_COMPILER_ID}'")
message("Build type is '${CMAKE_BUILD_TYPE}'")
message("IS_MSVC=${IS_MSVC}")
message("IS_CLANGCL=${IS_CLANGCL}")
message("IS_CLANG=${IS_CLANG}")
message("IS_MSVC_OR_CLANGCL=${IS_MSVC_OR_CLANGCL}")
message("IS_64BITS=${IS_64BITS}")
if(CXX STREQUAL "")
  message("CXX environment variable not defined")
else()
  message("CXX=${CXX}")
endif()
message("")

# MSVC compile options
if(IS_MSVC_OR_CLANGCL)
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

  # set warnings flag
  add_compile_options(/WX)

  # silence problematic warnings 
    # C4706 "assignement within conditional expression"
    #   Reason: This construct is used a lot in the code
    add_compile_options(/wd4706) 
  
    # C4324 "structure was padded due to alignement specifier"
    # C4458 "declaration of x hides class member"
    #   Reason: That's usually the intended behaviour
    add_compile_options(/wd4324 /wd4458)
  
    # C4291 "no matching operator delete found; memory will not be freed if initialization fails"
    #   Reason: This error happens for the operator new overloads of the AST, which allocate memory
    #           in the ASTContext, and we can't free a single block of memory without freeing everything.
    add_compile_options(/wd4291)

    # C4244 "conversion from 'a' to 'b', possible loss of data"
    #   Reason: This warning only exist on MSVC. 
    add_compile_options(/wd4244)

    # C4505 "unreferenced local function has been removed"
    #   Reason: Usually intended because of compile time checks 
    #           e.g; "checkHasGetRange" in lib/AST/Stmt.cpp
    add_compile_options(/wd4505)

    # C4276 "conversion from X to Y, possible loss of data"
    #   Reason: Disabled only in 64bits mode because it becomes problematic in some 
    #           LLVM libraries due to conversion from size_t to unsigned. (since size_t
    #           is larger in 64 bits mode).
    if(IS_64BITS)
      add_compile_options(/wd4267)
    endif()
  
  # don't deprecate theses functions
  add_compile_options(-D_SCL_SECURE_NO_WARNINGS)

  # TODO: Disable exceptions again. Unfortunately utf8_cpp needs them.
  #     When they're disabled again, nonstd/string-view will need some modifications,
  #     or a nssv_CONFIG_NO_EXCEPTIONS macro defined to 1 so it works with exceptions 
  #     disabled directly.
  # disable exceptions
  # add_compile_options(/D_HAS_EXCEPTIONS=0)
  # MSVC: Remove exceptions using /EHs-c-, but first remove /EHsc if present
  # if(CMAKE_CXX_FLAGS MATCHES "/EHsc")
  #  string(REGEX REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  # endif()
  # add_compile_options(/EHs-c-)

  # disable rtti
  add_compile_options(/GR-)
endif()

# Clang compile options 
if(IS_CLANG) 
  # for clang-cl, we must pass -Xclang when passing an argument.
  function(add_clang_option arg)
    if(IS_CLANGCL)
      add_compile_options("SHELL:-Xclang ${arg}")
    else() 
      add_compile_options(${arg})
    endif()
  endfunction()

  # sometimes, we need options to be passed only if the compiler
  # is clang, not clang-cl!
  # e.g. if /GR- has already been passed, we should pass -fno-rtti
  function(add_clangonly_option arg)
    if(NOT IS_CLANGCL)
      add_compile_options("${arg}")
    endif()
  endfunction()
  

  add_clang_option(-Wno-unused-parameter)

  # don't add -Wall for clang-cl, as it maps to -Weverything.
  # /W4 is used above instead.
  add_clangonly_option(-Wall)

  # TODO: Disable exceptions again. Unfortunately utf8_cpp needs them.
  # add_clangonly_option(-fno-exceptions)
  add_clangonly_option(-fno-rtti)

  # TODO: Fix warnings generated by Wreorder then remove this 
  add_clang_option(-Wno-reorder)
endif()

# add include paths 
include_directories(includes)               # Fox library includes
include_directories(${thirdparty_includes}) # Third Party libraries includes

# before disabling MSVC extensions, include the CommandLine and unittest
# folders, because both of them need MSVC extensions enabled.
add_subdirectory(tools/CommandLine)

# add unittests (creates the unittest target)
add_subdirectory(unittests)

# Disables extensions on MSVC 
if(IS_MSVC)
  add_compile_options(/Za)
  add_compile_options(/permissive-)
endif()

# add lib subdir (to define libfox_src)
add_subdirectory(lib)


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
