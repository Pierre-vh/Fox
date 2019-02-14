# this file contains the build script for the googletest project.
# including this file will do several things:
#   1 - Create a googletest target library (which builds gtest-all.cc)
#   2 - Create a "googletest_includes" variable with the path to the googletest
#       include folder.

# create the googletest library target
add_library(googletest STATIC 
            "${CMAKE_CURRENT_LIST_DIR}/googletest/src/gtest-all.cc")
# note: gtest-alL.cc needs to have the googletest folder as an include directory
target_include_directories(googletest PRIVATE
                           "${CMAKE_CURRENT_LIST_DIR}/googletest/")

# create the googletest_includes variable
set(googletest_includes "${CMAKE_CURRENT_LIST_DIR}/googletest/include")