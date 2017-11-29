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
	
	Lexer l;
	l.lexStr(content);

	if (E_CHECKSTATE)
	{
		l.iterateResults([](const token &t){
			E_LOG(t.showFormattedTokenData())
		});
	}
	Parser p(&l);
	auto ae = p.parseExpr();
	if (E_CHECKSTATE)
	{
		ae->accept(new Dumper());
	}
	else
		E_ERROR("Parsing failed.");

	std::cin.get();
	return 0;
}