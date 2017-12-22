#include "Symbols.h"

Symbols::Symbols()
{
}


Symbols::~Symbols()
{
}

Symbols & Symbols::getInstance()
{
	static Symbols instance;
	return instance;
}
