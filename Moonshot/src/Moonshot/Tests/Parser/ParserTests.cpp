#include "ParserTests.hpp"

#include "Moonshot/Common/Context/Context.hpp"
#include "Moonshot/Tests/Utils/Utils.hpp"
#include "Moonshot/Fox/Lexer/Lexer.hpp"
#include "Moonshot/Fox/Parser/Parser.hpp"
#include "Moonshot/Fox/AST/Nodes/ASTExpr.hpp"

using namespace Moonshot;
using namespace Moonshot::Tests;

ParserTests::ParserTests()
{
	// todo, populate tests map using the .def file
}

bool ParserTests::runTests(std::ostream & out, bool condensed)
{
	out << "Parser tests:\n";
	if (/* condition */ true)
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

ParserTests::IndividualTest::IndividualTest(const bool & should_pass, const ReadMode & rm, const std::string & fp, std::function<void(Parser&)> fn):
	shouldPass(should_pass), readmode(rm), filepath(fp), testfn(fn)
{

}
