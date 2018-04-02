#include <iostream>

#include "Moonshot/Driver/Driver.hpp"

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

using namespace Moonshot;

int main()
{
	setConsoleEnv();

	std::string uinput = "";
	while (1)
	{
		std::cout << "> Give me a file path, or enter * to exit: ";
		Driver drv;
		std::getline(std::cin, uinput);
		if (uinput == "*")
			break;

		drv.compileFunction(std::cout, uinput);
	}
	std::cout << "\n\nFinished. Press any key to continue.\n";
	std::cin.get();
	return 0;
}
