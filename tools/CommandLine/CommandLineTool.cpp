//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : CommandLineTool.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This is the entry point of the command line tool.
//----------------------------------------------------------------------------//

#include <iostream>
#include "Fox/Driver/Driver.hpp"
#include "Fox/Common/Version.hpp"
#ifdef _WIN32
  #include <Windows.h>
#endif

// Check if we can leak-check using _Crt leak-checking tools
#if defined(_MSC_VER) && !defined(_NDEBUG)
  #define CAN_LEAK_CHECK_ON_MSVC 1
  #define _CRTDBG_MAP_ALLOC  
  #include <stdlib.h>  
  #include <crtdbg.h>  
#else
  #define CAN_LEAK_CHECK_ON_MSVC 0
#endif 

void setConsoleEnv() {
  #ifdef _WIN32
    //  Windows-Specific stuff
    SetConsoleOutputCP(CP_UTF8);
    setvbuf(stdout, nullptr, _IOFBF, 1000);
  #endif
  // We don't use printf, so we don't need to sync with stdio (CppCoreGuidelines SL.io.10)
  std::ios_base::sync_with_stdio(false); 
}

using namespace fox;

int interactiveMain() {
  std::cout << "Fox Version " << FOX_VERSION_COMPLETE << "\n";
  std::cout << "\tUsage : Enter a path to a source file, or enter * to exit.\n\n";

  std::string uinput = "";
  bool res = true;
  while (1) {
    std::cout << "> ";
    std::getline(std::cin, uinput);
    if (uinput == "*")
      break;
    Driver drv(std::cout);
    drv.setDumpAST(true);
    res = drv.processFile(uinput);
  }
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

int cliMain(int argc, char *argv[]) {
  return Driver(std::cout).doCL(argc, argv) ? EXIT_SUCCESS : EXIT_FAILURE;
}

int main(int argc, char *argv[]) {
  // On MSVC, setup the leak-checking tool.
  #if CAN_LEAK_CHECK_ON_MSVC
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
  #endif
  setConsoleEnv();
  if (argc > 1) 
    return cliMain(argc, argv);
  return interactiveMain();
}
