# append_source_to_list
macro(append_source variable_name)
  foreach(source_file ${ARGN})
    list(APPEND ${variable_name} "${CMAKE_CURRENT_SOURCE_DIR}/${source_file}")
  endforeach()
endmacro()

# add_source
macro(add_source var_name)
  # add to list 
  append_source(${var_name} ${ARGN})
  # propagate the variable in parent directories
  set(${var_name} ${${var_name}} PARENT_SCOPE)
endmacro()

# add source dir
# this macro adds every subdir to the requested variable
macro(add_source_dir dest_var)
  foreach(sourcedir ${ARGN})
    add_subdirectory(${sourcedir})
    string(TOLOWER ${sourcedir} smallcase)
    set(final "${smallcase}_src")
    set(${dest_var} ${${dest_var}} ${${final}})
  endforeach()
  set(${dest_var} ${${dest_var}} PARENT_SCOPE)
endmacro()