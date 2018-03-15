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

			bool runTests(std::ostream& out,bool condensed = false);

		private:
			bool runCorrectTests(std::ostream& out, bool condensed);
			bool runIncorrectTests(std::ostream& out, bool condensed);
			/*
				Each file to test is put in this vector.
				Files are read into a string and that string is passed to the lexer.
			*/
			#define LEXER_TEST_CORRECT(FILE) FILE,
			std::vector<std::string> correctFiles_ = {
				#include "../def/lexer.def"
			};

			#define LEXER_TEST_INCORRECT(FILE) FILE,
			std::vector<std::string> incorrectFiles_ = {
				#include "../def/lexer.def"
			};
	};
}


