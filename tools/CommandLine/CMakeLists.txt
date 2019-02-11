add_executable(fox "CommandLineTool.cpp")
target_link_libraries(fox libfox)

# For MSVC, we must use /Ze because we're including files such as winrt.h, and 
# they use language extensions
if(IS_MSVC)
  if(CMAKE_CXX_FLAGS MATCHES "/Za")
    string(REGEX REPLACE "/Za" "/Ze" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    message("replaced")
  else()
    add_compile_options(/Ze)
  endif()
endif()

# print the compile options used (for debug purposes)
get_directory_property(compile_opts COMPILE_OPTIONS)
message("CommandLine Compile options used : ${compile_opts}")