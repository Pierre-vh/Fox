#include <iostream>
#include "src\Moonshot\Common\Errors\Errors.h"
#include "src\Moonshot\Fox\Lexer\Token.h"

using namespace Moonshot;


int main()
{
	E_LOG("Starting..");
	Errors *x = Errors::getInstance();

	std::cin.get();
	return 0;
}