////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTFunctionDeclaration.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "ASTFunctionDeclaration.hpp"
#include <stdexcept>
using namespace Moonshot;

ASTFunctionDeclaration::ASTFunctionDeclaration(const fn::FunctionSignature & funcsign, std::unique_ptr<ASTCompStmt> funcbody)
{
	if (!signature_)
		throw std::invalid_argument("the function signature provided is invalid/ill-formed.");
	
	signature_ = funcsign;
	body_ = std::move(funcbody);
}
