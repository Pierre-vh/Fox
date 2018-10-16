#include src to access ${ftt_src}
add_subdirectory(src)

#set include dirs
include_directories(includes)

#create the library
add_library(libftt STATIC ${ftt_src})

#link it with libmooshot
target_link_libraries(libftt libmoonshot)