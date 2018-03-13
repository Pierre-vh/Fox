////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : LexerMainTest.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "LexerMainTest.hpp"

using namespace Moonshot;
using namespace Moonshot::Test;
using namespace Moonshot::Test::TestUtilities;

bool LexerMainTest::runTest(Context & context)
{
	auto correct_test_str = readFileToString(context,"res\\tests\\lexer\\lexer_correct.fox");
	auto bad_test_str = readFileToString(context, "res\\tests\\lexer\\lexer_bad.fox");

	if (!context.isSafe())
		return false;
	// Run correct tests 
	Lexer lex_corr(context);
	lex_corr.lexStr(correct_test_str);

	if (!context.isSafe()) // Should be 0 errors. return false if one is found
		return false;

	lex_corr.logAllTokens();
	context.printLogs();
	context.clearLogs(); // clear logs of all the clutter

	// Run "bad" tests
	Lexer lex_bad(context);
	lex_bad.lexStr(bad_test_str); 
	if (context.isSafe()) // Should have errors
		return false;
	// All went well, reset context state and return.
	context.resetState();
	return true;
}

std::string LexerMainTest::getTestName() const
{
	return "Lexer Main";
}

