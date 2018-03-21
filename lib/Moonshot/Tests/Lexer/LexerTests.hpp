////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : LexerTests.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file implements the lexer tests, using lexer.def to generate them.										
////------------------------------------------------------////

#pragma once

#include <vector>
#include <string>
#include "../Utils/Utils.hpp"

namespace Moonshot::Tests
{
	class LexerTests
	{
		public:
			LexerTests() = default;

			bool runTests(std::ostream& out,const bool& condensed = false);

		private:
			bool runCorrectTests(std::ostream& out,const bool& condensed);
			bool runIncorrectTests(std::ostream& out,const bool& condensed);
			/*
				Each file to test is put in this vector.
				Files are read into a string and that string is passed to the lexer.
			*/
			#define LEXER_TEST_CORRECT(FILE) FILE,
			std::vector<std::string> correctFiles_ = {
				#include "Moonshot/Tests/def/lexer.def"
			};

			#define LEXER_TEST_INCORRECT(FILE) FILE,
			std::vector<std::string> incorrectFiles_ = {
				#include "Moonshot/Tests/def/lexer.def"
			};
	};
}


