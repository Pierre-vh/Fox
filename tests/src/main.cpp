////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : main.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Entry point of the Test executable.
////------------------------------------------------------////

#include "gtest/gtest.h"
#include <iostream>

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	auto res = RUN_ALL_TESTS();
	std::cin.get();
	return res;
}