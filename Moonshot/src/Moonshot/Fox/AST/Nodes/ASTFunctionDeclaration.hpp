////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTFunctionDeclaration.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// The AST Node for Function Declarations										
////------------------------------------------------------////

#pragma once
#include "IASTNode.hpp"
#include "ASTCompStmt.hpp"
#include "Moonshot/Fox/Common/FunctionSignature.hpp"

namespace Moonshot
{
	struct ASTFunctionDeclaration : public IASTNode	
	{
		ASTFunctionDeclaration() = default;
		ASTFunctionDeclaration(const fn::FunctionSignature &funcsign, std::unique_ptr<ASTCompStmt> funcbody);

		virtual void accept(IVisitor& vis) { vis.visit(*this); }

		std::unique_ptr<ASTCompStmt> body_;
		fn::FunctionSignature signature_;
	};
}