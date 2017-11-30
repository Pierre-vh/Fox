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
#include <fstream>
#include <string>
#include <typeinfo.h>
using namespace Moonshot;

int main()
{
	E_LOG("Starting..");
	Errors *x = Errors::getInstance();

	//std::ifstream ifs("C:\\Users\\pierr\\Source\\Repos\\Moonshot---WIP\\Moonshot\\res\\code_samples\\expr_test_secondops.fox");
	std::ifstream ifs("C:\\Users\\pierre.vanhoutryve\\source\\repos\\Moonshot\\Moonshot\\res\\code_samples\\expr_test_secondops.fox");
	if (!ifs)
		E_CRITICAL("Failed to open file ")
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