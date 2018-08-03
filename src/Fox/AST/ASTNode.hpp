////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTNode.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// The ASTNode is a class that acts like a variant of Expr/Stmt/Decl pointers.
////------------------------------------------------------////

#include "Fox/Common/PtrVariant.hpp"
#include "Expr.hpp"
#include "Decl.hpp"
#include "Stmt.hpp"

namespace fox
{
	class ASTNode : public PtrVariant<Expr, Stmt, Decl>
	{
		public:
			using PtrVariant::PtrVariant;

			SourceRange getRange() const;
			SourceLoc getBegLoc() const;
			SourceLoc getEndLoc() const;
	};
}