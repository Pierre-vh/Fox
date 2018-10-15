////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : CommandLineTool.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This is the entry point of the command line tool.
// As of April 2018, it is still extremely basic, and only meant for testing/parser demonstration purposes.
////------------------------------------------------------////

#include <iostream>

#include "Fox/Driver/Driver.hpp"
#include "Fox/Common/Version.hpp"

#ifdef _WIN32
	#include <Windows.h>
#endif

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

using namespace fox;

int main()
{
	setConsoleEnv();
	std::cout << "Welcome to the Dumb Command Line Toy !\n\tMoonshot Version " << MOONSHOT_VERSION_COMPLETE << "\n";
	std::cout << "\tUsage : Enter a path to a fox source file, or enter * to exit.\n\n";

	std::string uinput = "";
	while (1)
	{
		std::cout << "> ";
		Driver drv;
		std::getline(std::cin, uinput);
		if (uinput == "*")
			break;

		drv.processFile(std::cout, uinput);
	}
	std::cout << "\n\nFinished. Press any key to continue.\n";
	std::cin.get();
	return 0;
}
