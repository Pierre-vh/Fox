#include <iostream>

#ifdef _WIN32
	#include <Windows.h>
#endif

// TODO:
/*
	Switch to CMake for good (make everything build as desired, and integrating with visual studio well)
	Add /tools/ folder which creates Moonshot.exe, rename /src/ to lib and make it create libmoonshot
	set(parser_srcs 
		<files>
	PARENT_SCOPE)

	set_property(GLOBAL APPEND PROPERTY ms_src {parser_srcs} ..)
	get_property(libmoon_sources GLOBAL PROPERTY ms_srcs)

	Do migration to googletests once that's done

		- ASTUnit -> a file, contains the import directives
			-> Once ASTUnit is in, finish the master rule and parser's done !
		- ASTContext/ASTRoot

		Rework the visitor pattern and adapt Dumper to support it. 
		Refactor the whole project.
			Take a peek at everything inside /Common/Types to rename stuff that needs it, make names more explicit. On top of my head:
				FoxValue can be deleted altogether, and just use different ASTLiteral nodes. 
				Look at everything that's not needed anymore an throw it in the trash too.
			Also, take a peek a the general Option system. Maybe let it use .def files too. 
			In general, mark stuff that should be reworked and delete things that need to be purged.

		Rework the visitor pattern, add ASTWalker, ASTVisitor, ASTTraversal. (use a clang/swift-like design pattern)

		Add declContext support (symbols table) + resolved/unresolved IDs
	
	ASTNamespaceDecl & Parser Driver:
		Primitive parser driver : Takes all the parsed ASTFiles and create a root node, ordering them by namespace (filepaths) to produce a full file.
	Recreate semantic analysis phase in a /sema/ folder
		- Name Resolver
		- Type Checker
		- Other general AST Validation / Semantic checks.
*/

void setConsoleEnv()
{
	#ifdef _WIN32
		//	Windows-Specific
		SetConsoleOutputCP(CP_UTF8);
		setvbuf(stdout, nullptr, _IOFBF, 1000);
	#endif
	std::ios_base::sync_with_stdio(false); // We don't use printf, so we don't need to sync with stdio (CppCoreGuidelines SL.io.10)
}

// reminder : use ifndef NDEBUG to know if debug/release mode
int main()
{
	setConsoleEnv();

	std::cout << "Finished. Press any key to continue.\n";
	std::cin.get();
	return 0;
}
