//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : main.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// Entry point of the Test executable.
//----------------------------------------------------------------------------//

#include "gtest/gtest.h"

// Check if we can leak-check using _Crt leak-checking tools
#if defined(_MSC_VER) && !defined(_NDEBUG)
  #define CAN_LEAK_CHECK_ON_MSVC 1
  #define _CRTDBG_MAP_ALLOC  
  #include <stdlib.h>  
  #include <crtdbg.h>  
#else
  #define CAN_LEAK_CHECK_ON_MSVC 0
#endif 

int main(int argc, char **argv) {
  // Enable leak checking under MSVC.
  #if CAN_LEAK_CHECK_ON_MSVC
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
  #endif
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}