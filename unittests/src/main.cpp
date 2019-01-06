//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : main.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// Entry point of the Test executable.
//----------------------------------------------------------------------------//

#include "gtest/gtest.h"
#include <iostream>

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  auto res = RUN_ALL_TESTS();
  std::cin.get();
  return res;
}