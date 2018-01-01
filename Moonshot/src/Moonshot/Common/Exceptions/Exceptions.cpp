#include "Exceptions.h"

using namespace Moonshot;

lexer_critical_error::lexer_critical_error(const std::string & msg) 
{
	msg_ += "\n" + msg;
}
