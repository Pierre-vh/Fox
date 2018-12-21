//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
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
  std::cout << "Moonshot Version " << MOONSHOT_VERSION_COMPLETE << "\n";
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
    drv.setVerifyModeEnabled(true);
    res = drv.processFile(uinput);
  }
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

int cliMain(int argc, char *argv[]) {
  return Driver(std::cout).doCL(argc, argv) ? EXIT_SUCCESS : EXIT_FAILURE;
}

int main(int argc, char *argv[]) {
  setConsoleEnv();
  if (argc > 1) 
    return cliMain(argc, argv);
  return interactiveMain();
}
