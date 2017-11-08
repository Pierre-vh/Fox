#include <iostream>
#include "src\Moonshot\Common\Errors\Errors.h"

using namespace Moonshot;


int main()
{
	E_LOG("Starting..");
	Errors *x = Errors::getInstance();
	if(x)
		std::cout << "HELLO" << std::endl;

	E_WARNING("BAD!");
	std::cout << (E_CHECKSTATE ? "OK" : "NOT OK") << std::endl;

	E_CRITICAL("OUCH OUCH OWIIIEE");
	std::cout << (E_CHECKSTATE ? "OK" : "NOT OK") << std::endl;

	std::cin.get();
	return 0;
}