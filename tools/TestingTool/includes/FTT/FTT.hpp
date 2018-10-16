////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : FTT.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file contains the main interface for the Fox Testing Tool.
// (TODO)
////------------------------------------------------------//// 

#pragma once

#include <iostream>
#include <string>

namespace fox
{
namespace ftt
{
	/*
		This class is unique for each file. It contains
		instance variables and test preferences set by the file, and
		methods to execute the test file.
	*/
	class FileTest
	{
		public:
			FileTest(const std::string& str, std::ostream& os = std::cout);
			// function to check the state "canContinue"
			// runNext() that returns struct
		private:

	};
}
}