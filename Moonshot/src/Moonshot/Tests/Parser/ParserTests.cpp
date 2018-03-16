////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ParserTests.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "ParserTests.hpp"

#include "Moonshot/Common/Context/Context.hpp"
#include "Moonshot/Tests/Utils/Utils.hpp"
#include "Moonshot/Fox/Lexer/Lexer.hpp"
#include "Moonshot/Fox/Parser/Parser.hpp"
#include "Moonshot/Fox/AST/Dumper/Dumper.hpp"
#include "Moonshot/Fox/AST/Nodes/ASTExpr.hpp"

using namespace Moonshot;
using namespace Moonshot::Tests;

/*
	Parser tests indent:
	0 : Test name : "Parser Tests"
	1 : Test Group : e.g. "Expressions"
	3 : Individual test result (<file:line>)
	2 : Test group result
	1 : Final result:  "ALL PARSER TESTS PASSED"
*/

template<typename Ty>
bool handleParsingResult(const ParsingResult<Ty>& pt, const bool& show_ast)
{
	// fix bug: should print the after the test is announced, not before
	if (pt && (pt.getFlag() != ParsingOutcome::NOTFOUND))
	{
		if (show_ast)
			pt.result_->accept(Dumper(4));
		return true;
	}
	return false;
}

ParserTests::ParserTests()
{
#define PARSER_TEST(SHOULDPASS,GROUPID,MODE,FILEPATH,FUNC) tests[ParserTestGroups::##GROUPID].tests.push_back(IndividualTest(SHOULDPASS,ReadMode::##MODE,FILEPATH,[&](Parser &p,const bool& printast){return handleParsingResult(p.##FUNC,printast);}));
	#include "Moonshot/Tests/def/parser.def"
}

bool ParserTests::runTests(std::ostream & out,const bool& condensed, const bool& show_ast_)
{
	showAST = show_ast_;

	out << "Parser tests:\n";
	return runAllTestsGroups(out, condensed);
}

bool ParserTests::runAllTestsGroups(std::ostream & out, const bool & condensed)
{
	bool successFlag = true;
	for (const auto& elem : tests)
	{
		out << indent(1) << elem.second.name << "\n";
		if (!runTestGroup(elem.second, out, condensed))
			successFlag = false;
	}
	return successFlag;
}

bool ParserTests::runTestGroup(const TestGroup & testgrp, std::ostream& out, const bool & condensed)
{
	bool successflag = true;
	for (auto& elem : testgrp.tests)
	{
		out << indent(3) << "File: \"" << elem.filepath << "\"\n";
		if (!runIndividualTest(elem, out, condensed))
			successflag = false;
	}
	if (successflag)
		out << indent(2) << "ALL TESTS IN \"" << testgrp.name << "\" GROUP PASSED\n";
	else 
		out << indent(2) << "ONE OR MORE TEST IN \"" << testgrp.name << "\" GROUP FAILED\n";
	return successflag;
}

bool ParserTests::runIndividualTest(const IndividualTest & curtest, std::ostream & out, const bool & condensed)
{
	// Put all tests in vec
	std::vector<std::string> tests_strs;
	if (curtest.readmode == ReadMode::LINES)
	{
		if (!readFileToVec("parser/" + curtest.filepath, tests_strs))
		{
			out << indent(3) << "Can't open test file \"" << curtest.filepath << "\". TEST FAILED\n";
			return false;
		}
	}
	else
	{
		std::string tmp;
		if (!readFileToString("parser/" + curtest.filepath, tmp))
		{
			out << indent(3) << "Can't open test file \"" << curtest.filepath << "\". TEST FAILED\n";
			return false;
		}
		else
			tests_strs.push_back(tmp);
	}
	bool successFlag = true;
	// run all tests in array
	int counter = 0;
	for (auto test : tests_strs)
	{
		counter++;

		if (test[0] == '#')
			continue;

		Context ctxt;
		ctxt.setLoggingMode(Context::LoggingMode::SAVE_TO_VECTOR);

		Lexer lex(ctxt);
		lex.lexStr(test);
		if (!ctxt.isSafe())
		{
			if (curtest.shouldPass)
			{
				successFlag = false;
				if(curtest.readmode == ReadMode::LINES)
					out << indent(4) << "Line " << counter << ". FAILED @ lEXING\n";
				else 
					out << indent(4) << "TEST FAILED @ lEXING\n";
				out << ctxt.getLogs();
			}
			else
			{
				if (curtest.readmode == ReadMode::LINES)
					out << indent(4) << "Line " << counter << ". PASSED (FAILED AS EXPECTED)\n";
				else 
					out << indent(4) << "TEST PASSED (FAILED AS EXPECTED)\n";
			}
			continue;
		}
		Parser par(ctxt, lex.getTokenVector());
		if (curtest.testfn(par, showAST) && ctxt.isSafe())
		{
			// Test successful & test should pass
			if (curtest.shouldPass)
			{
				if (!condensed)
				{
					if (curtest.readmode == ReadMode::LINES)
						out << indent(4) << "Line " << counter << ". PASSED\n";
					else
						out << indent(4) << "TEST PASSED\n";
				}
			}
			// Test successful & test should have failed
			else 
			{
				successFlag = false;
				if (curtest.readmode == ReadMode::LINES)
					out << indent(4) << "Line " << counter << ". FAILED (TEST EXPECTED TO FAIL BUT PASSED)\n";
				else
					out << indent(4) << "TEST FAILED (TEST EXPECTED TO FAIL BUT PASSED)\n";
				out << ctxt.getLogs();
			}
		}
		else
		{
			// Test failed & test should have passed
			if (curtest.shouldPass)
			{
				successFlag = false;
				if (curtest.readmode == ReadMode::LINES)
					out << indent(4) << "Line " << counter << ". FAILED\n";
				else
					out << indent(4) << "TEST FAILED\n";
				out << ctxt.getLogs();
			}
			// test failed & test should have failed
			else
			{
				if (curtest.readmode == ReadMode::LINES)
					out << indent(4) << "Line " << counter << ". PASSED (FAILED AS EXPECTED)\n";
				else
					out << indent(4) << "TEST PASSED (FAILED AS EXPECTED)\n";
			}
		}
	}
	return successFlag;
}

ParserTests::IndividualTest::IndividualTest(const bool & should_pass, const ReadMode & rm, const std::string & fp, std::function<bool(Parser&,const bool&)> fn):
	shouldPass(should_pass), readmode(rm), filepath(fp), testfn(fn)
{

}

ParserTests::TestGroup::TestGroup(const std::string & nm,const std::vector<IndividualTest> tsts) :
	name(nm), tests(tsts)
{

}
