////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : LexerTests.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "LexerTests.hpp"

#include "Moonshot/Common/Context/Context.hpp"
#include "Moonshot/Tests/Utils/Utils.hpp"

#include "Moonshot/Fox/Lexer/Lexer.hpp"

using namespace Moonshot;
using namespace Moonshot::Tests;

bool LexerTests::runTests(std::ostream & out,const bool& condensed)
{
	out << "Lexer tests:\n";
	if (runCorrectTests(out, condensed) && runIncorrectTests(out, condensed))
	{
		out << indent(1) << "ALL TESTS PASSED\n";
		return true;
	}
	else
	{
		out << indent(1) << "ONE OR MORE TESTS FAILED\n";
		return false;
	}
}

bool LexerTests::runCorrectTests(std::ostream & out,const bool& condensed)
{
	bool successFlag = true;
	int counter = 0;
	for (const auto& file : correctFiles_)
	{
		counter++;

		std::string content;
		if (!readFileToString("lexer/" + file, content))
		{
			out << indent(2) << counter << ". FAILED: Could not open file \"" << file << "\"";
			successFlag = false;
			continue;
		}
		Context ctxt;
		Lexer lex(ctxt);
		lex.lexStr(content);
		if (ctxt.isSafe())
		{
			// test success. if !condensed, print details
			if (!condensed)
				out << indent(2) << counter << ". \"" << file << '"' << indent(2) << "PASSED\n";
			continue;
		}
		else
		{
			successFlag = false;
			// test failed
			out << indent(2) << counter << ". \"" << file << '"' << indent(2) << "FAILED. Context log:\n";
			out << ctxt.getLogs() << '\n';

		}
	}
	return successFlag;
}

bool LexerTests::runIncorrectTests(std::ostream & out,const bool& condensed)
{
	bool successFlag = true;
	int counter = -1;
	for (const auto& file : incorrectFiles_)
	{
		counter++;

		std::string content;
		if (!readFileToString("lexer/" + file, content))
		{
			out << indent(2) << counter << ". FAILED: Could not open file \"" << file << "\"";
			successFlag = false;
			continue;
		}
		Context ctxt;
		ctxt.setLoggingMode(Context::LoggingMode::SAVE_TO_VECTOR);

		Lexer lex(ctxt);
		lex.lexStr(content);
		if (!ctxt.isSafe())
		{
			// test success. if !condensed, print details
			if (!condensed) 
				out << indent(2) << counter << ". \"" << file << '"' << indent(2) << "PASSED\n";
			continue;
		}
		else
		{
			successFlag = false;
			// test failed
			out << indent(2) << counter << ". \"" << file << '"'  << indent(2) << "FAILED. (Test successful but was expected to fail)\n";
			out << ctxt.getLogs() << '\n';

		}
	}
	return successFlag;
}
