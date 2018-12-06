add_executable(Moonshot "CommandLineTool.cpp")
target_link_libraries(Moonshot libmoonshot)
add_msvc_compile_options(/Ze)