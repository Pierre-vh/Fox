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
    //  Windows-Specific
    SetConsoleOutputCP(CP_UTF8);
    setvbuf(stdout, nullptr, _IOFBF, 1000);
  #endif
  std::ios_base::sync_with_stdio(false); // We don't use printf, so we don't need to sync with stdio (CppCoreGuidelines SL.io.10)
}

// reminder : use ifndef NDEBUG to know if debug/release mode

using namespace fox;

int interactiveMain() {
  std::cout << "Welcome to the Dumb Command Line Toy !\n\tMoonshot Version " << MOONSHOT_VERSION_COMPLETE << "\n";
  std::cout << "\tUsage : Enter a path to a fox source file, or enter * to exit.\n\n";

  std::string uinput = "";
  bool res = true;
  Driver drv(std::cout);
  //drv.setDumpAlloc(true);
  drv.setDumpAST(true);
  // Test of the diagnostic verifier, this line will be removed later.
  drv.setVerifyMode(Driver::VerifyMode::Normal);
  //drv.setPrintChrono(true);
  while (1) {
    std::cout << "> ";
    std::getline(std::cin, uinput);
    if (uinput == "*")
      break;

    res &= drv.processFile(uinput);
  }
  std::cout << "\n\nFinished. Press any key to continue.\n";
  std::cin.get();
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
