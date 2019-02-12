# Including this file will define the thirdparty_includes variable
#
# thirdparty_includes contains a list of the include paths of 
# every third-party library needed by the Fox project's main
# librarieS.

# add_thirdparty_include_folder
#   Adds a relative include path to the thirdparty_includes variable.
macro(add_thirdparty_include_folder relative_path)
  # add the path, but make it an absolute path
  list(APPEND thirdparty_includes 
              "${CMAKE_CURRENT_LIST_DIR}/${relative_path}")
endmacro()

##################################################################
# Add new third-party include paths here. Currently, the only 
# third-party libraries used are header only, so there's no way 
# to add third-party build targets
##################################################################

add_thirdparty_include_folder("")

##################################################################

# check that we have correctly defined thirdparty_includes and 
# that it's not empty
if(NOT thirdparty_includes)
  message(SEND_ERROR 
    "third-party includes list is empty, or has not been defined.\
     thirdparty_includes=${thirdparty_includes}")
endif()