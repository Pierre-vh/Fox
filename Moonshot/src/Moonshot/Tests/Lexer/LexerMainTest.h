////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : LexerMainTest.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Main lexer test.
// This runs the Lexer, with "tests/lexer/lexer_bad.fox" and "lexer_good.fox".
// This class does not use the parser.
////------------------------------------------------------////

#pragma once

#include "../ITest.h"

namespace Moonshot
{
	class LexerMainTest : public ITest
	{
		public:
			LexerMainTest() = default;
			~LexerMainTest();

			// Inherited via ITest
			virtual bool runTest(Context & context) override;
			virtual std::string getTestName() const override;
	};
}

