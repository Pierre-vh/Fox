#include <iostream>
#include "src\Moonshot\Fox\AST\Visitor\Dumper\Dumper.h"
#include "src\Moonshot\Common\FValue\FValue.h"
#include "src\Moonshot\Common\Errors\Errors.h"
#include "src\Moonshot\Fox\Lexer\Token.h"
#include "src\Moonshot\Fox\Lexer\Lexer.h"
#include "src\Moonshot\Fox\Parser\Parser.h"
#include "src\Moonshot\Fox\AST\Nodes\ASTExpr.h"
#include "src\Moonshot\Fox\AST\Nodes\IASTNode.h"
#include <variant>
#include <sstream>
#include <fstream>
#include <string>
using namespace Moonshot;

int main(int argc, char* argv[])
{
	E_LOG("Starting..");
	Errors *x = Errors::getInstance();
	std::ifstream ifs(util::filepath_MoonshotProj("tests\\expr\\expr.fox"));
	if (!ifs)
	{
		std::stringstream ss;
		ss << "Failed to open file from " << argv[0];
		E_CRITICAL(ss.str())
	}
	std::string content((std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>()));
	std::cout << "Input : \n\t" << char(192) << "-> " << content << std::endl;

	Lexer l;
	l.lexStr(content);
	Parser p(&l);
	auto ae = p.parseExpr();
	if (E_CHECKSTATE)
		ae->accept(new Dumper());
	else
		E_ERROR("Parsing failed. Can't print the tree when the parser isn't in a healthy state.");
	std::cin.get();
	return 0;
}