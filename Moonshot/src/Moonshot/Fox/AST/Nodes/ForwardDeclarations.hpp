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
	struct IASTStmt;
	struct IASTExpr;
	struct IASTDecl;

	struct ASTCompoundStmt;
	struct ASTCondStmt;
	struct ASTWhileStmt;
	struct ASTReturnStmt;
	struct ASTNullStmt;

	struct ASTBinaryExpr;
	struct ASTUnaryExpr;
	struct ASTCastExpr;
	struct ASTLiteralExpr;
	struct ASTDeclRefExpr;

	struct ASTFunctionDecl;
	struct ASTVarDecl;

	struct FoxFunctionArg;
}