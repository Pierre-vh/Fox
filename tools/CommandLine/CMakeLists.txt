add_executable(fox "CommandLineTool.cpp")
target_link_libraries(fox libfox)
add_msvc_compile_options(/Ze)