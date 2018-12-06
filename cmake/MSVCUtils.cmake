# Adds a compile option only if the current compiler is MSVC
macro(add_msvc_compile_options option1)
  if(MSVC)
    add_compile_options(option1 ${ARGN})
  endif()
endmacro()