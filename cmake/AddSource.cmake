# converts val to a variable name, puts the result in the_var
# e.g. foo becomes foo_src
macro(to_varname val the_var)
  string(TOLOWER "${val}_src" ${the_var})
endmacro()

# converts a directory to a variable name, puts the result in the_var
# e.g. foo/bar/fox/lib/AST/ becomes ast_src
macro(dir_to_varname dir the_var)
  # get the directory's name
  get_filename_component(dirname ${dir} NAME)
  # call to_varname
  to_varname(${dirname} ${the_var})
endmacro()

# append_source_to_list
macro(append_to_list variable_name)
  foreach(source_file ${ARGN})
    list(APPEND ${variable_name} "${CMAKE_CURRENT_SOURCE_DIR}/${source_file}")
  endforeach()
endmacro()

# add_source
macro(add_source)
  # The base of the variable name is the name of the folder we're in
  dir_to_varname(${CMAKE_CURRENT_SOURCE_DIR} the_var)
  # add the source files to the list
  append_to_list(${the_var} ${ARGN})
  # propagate the variable in parent directories
  set(${the_var} ${${the_var}} PARENT_SCOPE)
endmacro()

# add source dir
# this macro adds every subdir to the requested variable
macro(add_source_dir dest_var)
  foreach(sourcedir ${ARGN})
    # add the subdir to import its variables
    add_subdirectory(${sourcedir})
    # get the varname
    to_varname(${sourcedir} var_name)
    set(${dest_var} ${${dest_var}} ${${var_name}})
  endforeach()
  set(${dest_var} ${${dest_var}} PARENT_SCOPE)
endmacro()