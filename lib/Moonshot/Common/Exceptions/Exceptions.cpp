////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Exceptions.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Exceptions.hpp"

using namespace Moonshot::Exceptions;

lexer_critical_error::lexer_critical_error(const std::string & msg) 
{
	msg_ += "\n" + msg;
}

parser_critical_error::parser_critical_error(const std::string & msg) : msg_(msg)
{
}

ast_malformation::ast_malformation(const std::string & msg) : msg_(msg)
{
}
