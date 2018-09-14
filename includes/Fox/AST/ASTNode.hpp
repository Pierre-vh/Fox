////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTNode.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// The ASTNode is a class that acts like a variant of Expr/Stmt/Decl pointers.
////------------------------------------------------------////

#pragma once
#include "Fox/Common/PtrVariant.hpp"

namespace fox
{
	class Expr;
	class Stmt;
	class Decl;
	class SourceRange;
	class SourceLoc;
	class ASTNode : public PtrVariant<Expr, Stmt, Decl>
	{
		public:
			using PtrVariant::PtrVariant;

			SourceRange getRange() const;
			SourceLoc getBegLoc() const;
			SourceLoc getEndLoc() const;

			// Common helpers
			bool isNullStmt() const;
	};
}