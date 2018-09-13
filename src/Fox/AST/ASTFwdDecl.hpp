////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTFwdDecl.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file does a forward declaration of every node in the AST.
////------------------------------------------------------////

namespace fox
{
	#define EXPR(ID, PARENT) class ID;
	#include "ExprNodes.def"

	#define DECL(ID, PARENT) class ID;
	#include "DeclNodes.def"

	#define STMT(ID, PARENT) class ID;
	#include "StmtNodes.def"

	#define TYPE(ID, PARENT) class ID;
	#include "TypeNodes.def"
}