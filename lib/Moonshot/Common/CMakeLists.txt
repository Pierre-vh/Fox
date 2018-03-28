add_source_dir(common_src Types)

# I'm adding some source files manually here, there's no point in having a CMakeLists in  
# a directory for just one file!
add_source(common_src 
	"Flags/Flags.cpp"
	"Context/Context.cpp"
	"Exceptions/Exceptions.cpp"
	)