#include <iostream>
#include "src\Moonshot\Fox\AST\Visitor\Semantic\TypeCheck.h"
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

#define	RTR_IF_ERR if(!(*(Moonshot::Errors::getInstance()))) {std::cin.get();return -1;}

int main(int argc, char* argv[])
{
	// Note: Main is mostly personal tests. It's not definitive logic, and will change with nearly every commit.
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

	RTR_IF_ERR
	Lexer l;
	l.lexStr(content);
	
	RTR_IF_ERR
	Parser p(&l);
	auto ae = p.parseExpr();
	if (E_CHECKSTATE)
		ae->accept(new Dumper());
	else
		E_ERROR("Parsing failed. Can't print the tree when the parser isn't in a healthy state.");

	RTR_IF_ERR
	TypeCheck *tc = new TypeCheck;
	ae->accept(tc); // Typecheck the tree
	if (E_CHECKSTATE)
	{
		std::cout << "The has been typechecked an no error were found. This expression will return a ";
		FVal v = tc->getReturnTypeOfExpr();
		auto typ = getTypeFromFVal(v);
		auto search = parse::kType_dict.find(typ);
		std::cout << search->second << std::endl;
	}
	else
		std::cout << "The tree has failed a typecheck." << std::endl;	
	std::cin.get();
	return 0;
}