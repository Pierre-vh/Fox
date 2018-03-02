////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ForwardDeclarations.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Contains forward declarations of every node/class in this folder
// to avoid having to include every single file in the parser/IVisitor classes.
////------------------------------------------------------////

#pragma once

namespace Moonshot
{
	struct IASTNode;
	struct IASTStmt;
	struct IASTExpr;
	struct IASTDeclaration;

	struct ASTLiteral;
	struct ASTVarDeclStmt;
	struct ASTVarCall;
	struct ASTCompStmt;
	struct ASTCondition;
	struct ASTWhileLoop;
	struct ASTBinaryExpr;
	struct ASTUnaryExpr;
	struct ASTCastExpr;
	struct ASTNullStmt;
	struct ASTFunctionDeclaration;
}