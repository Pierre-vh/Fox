# add_to_list
function(add_to_list variable_name)
  #variable_name holds the name of the variable that we need to set
  #so ${variable} == name, ${${variable}} == contents of said variable
  set(srcs )
  foreach(source_file ${ARGN})
    list(APPEND srcs "${CMAKE_CURRENT_SOURCE_DIR}/${source_file}")
  endforeach()
  #Functions have local scope so we need to set the callers version of the
  #variable to what our version has.
  #Note the double dereference to get the contents of
  #the variable whose name we are given
  set(${variable_name} ${${variable_name}} ${srcs} PARENT_SCOPE)
endfunction()


# add_source
macro(add_source var_name)
  # add to list 
  add_to_list(${var_name} ${ARGN})
  # propagate
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