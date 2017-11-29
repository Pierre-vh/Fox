#include <iostream>
#include "src\Moonshot\Common\Errors\Errors.h"
#include "src\Moonshot\Fox\Lexer\Token.h"
#include "src\Moonshot\Fox\Lexer\Lexer.h"
#include "src\Moonshot\Fox\Parser\Parser.h"
#include "src\Moonshot\Fox\Parser\AST\ASTNode.h"
#include "src\Moonshot\Fox\Parser\AST\ASTExpr.h"
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
	ae->showTree();
	std::cin.get();
	return 0;
}