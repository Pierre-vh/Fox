# add_source
#   append the absolute paths of the source files passed to the variable 
#   "the_var"
macro(add_source the_var)
  # append source files to the variable 
  foreach(source_file ${ARGN})
    list(APPEND ${the_var} "${CMAKE_CURRENT_SOURCE_DIR}/${source_file}")
  endforeach()
  # propagate the variable in the parent scope
  set(${the_var} ${${the_var}} PARENT_SCOPE)
endmacro()