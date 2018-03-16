#include <iostream>
#include <Windows.h>
#include "Tests/Lexer/LexerTests.hpp"
#include "Tests/Parser/ParserTests.hpp"

using namespace Moonshot;

// Add tests for member access + dumper support to check if I didn't messed up while 
// coding that (i probably did, it's just a matter to find where :))) )
// Then move on to imports & master rule and parser's nearly done. I'll spend a few hours
// tweaking it and improving it before moving on to the ParserDriver

/*
	This is a short term roadmap for the project.
		Implement the rest of the Parsing functions & AST Structure
			- ASTUnit -> a file, contains the import directives
				-> Once ASTUnit is in, finish the master rule and parser's done !
			- ASTContext/ASTRoot

		Rework the visitor pattern and adapt Dumper to support it. 
		Refactor the whole project.
			Delete /eval/ folder. The code contained in that is complete trash.
			Delete the typecheck class in prevision of upcoming rewrite.
			Delete /datamap/, it's filth.
			Take a peek at everything inside /Common/Types to rename stuff that needs it, make names more explicit. On top of my head:
				FoxValue can be deleted altogether, and just use different ASTLiteral nodes. 
				Look at everything that's not needed anymore an throw it in the trash too.
			Also, take a peek a the general Option system. Maybe let it use .def files too. 
			It should be really easy to add new options and Retrieve options. Not a pain in the ass as it is currently.
				Options should all have int values.
				Options should be easy to add and retrieve.
			In general, mark stuff that should be reworked and delete things that need to be purged.

			After that, the project will be able to start again on healthier grounds. This shouldn't be a huge setback, I'm
			just throwing stuff in the trash to avoid building on it. I prefer to rewrite 2 to 3k sloc now than 10 to 30 later.

			I'm also thinking about switching to a CMake build system and start coding on Linux a bit more.
			This will make the project better in the long run I think (more people open to contribute, easier to build the project,etc)

		Rework the visitor pattern, add ASTWalker, ASTVisitor, ASTTraversal. (use a clang/swift-like design pattern)
			Quick thought: I might rework the AST a little bit, with removing access to the raw data and only use helper functions. This will
			be decided at this step. It would require some modification to the parser and dumper, but nothing too hard. This would allow
			for more control over the AST and it's behaviour.

		Rewrite the whole test system, possibily by using .def files to automate their creation. I'll need to think about that a lot!
			- Separate tests in categories : lexer, parser, semantics, ...
			- Add more control over tests individually: should it print the AST, should it just print "PASSED/FAILED", show them in condensed form, etc.

		
		Add declContext support (symbols table) + resolved/unresolved IDs
	
	ASTNamespaceDecl & Parser Driver:
		Primitive parser driver : Takes all the parsed ASTFiles and create a root node, ordering them by namespace (filepaths) to produce a full file.
	Recreate semantic analysis phase in a /sema/ folder
		- Name Resolver
		- Type Checker
		- Other general AST Validation / Semantic checks.
	After that, (it's very far from now!) start doing the IR gen phase.
*/

int main()
{
	SetConsoleOutputCP(CP_UTF8);
	setvbuf(stdout, nullptr, _IOFBF, 1000);

	std::ios_base::sync_with_stdio(false); // We don't use printf, so we don't need to sync with stdio (CppCoreGuidelines SL.io.10)

	Tests::LexerTests lt;
	lt.runTests(std::cout);

	Tests::ParserTests pt;
	pt.runTests(std::cout,false,true);

	std::cout << "Finished. Press any key to continue.\n";
	std::cin.get();
	return 0;
}
