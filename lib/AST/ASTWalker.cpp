////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTWalker.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Fox/AST/ASTWalker.hpp"
#include "Fox/AST/ASTVisitor.hpp"
#include "Fox/Common/Errors.hpp"

using namespace fox;

namespace
{
	class Traverse : public ASTVisitor<Traverse, Decl*, Expr*, Stmt*, Type*>
	{
		private:
			// Implementation of ASTWalker
				// visit methods must return a node, which is the current node or
				// or the replacement. That node cannot be null.
			
		public:
			Traverse(ASTWalker& walker):
				walker_(walker)
			{}

		private:
			ASTWalker& walker_;
	};
} // End anonymous namespace

ASTNode ASTWalker::walk(ASTNode node)
{
	if (auto decl = node.getIf<Decl>())
		return walk(decl);
	if (auto stmt = node.getIf<Stmt>())
		return walk(stmt);
	if (auto expr = node.getIf<Expr>())
		return walk(expr);

	fox_unreachable("Unknown node contained in ASTNode");
}

Expr* ASTWalker::walk(Expr* expr)
{
	return Traverse(*this).visit(expr);
}

Decl* ASTWalker::walk(Decl* decl)
{
	return Traverse(*this).visit(decl);
}

Stmt* ASTWalker::walk(Stmt* stmt)
{
	return Traverse(*this).visit(stmt);
}

Type* ASTWalker::walk(Type* type)
{
	return Traverse(*this).visit(type);
}

std::pair<bool, Expr*> ASTWalker::handleExprPre(Expr* expr)
{
	return { true, expr };
}

Expr* ASTWalker::handleExprPost(Expr* expr)
{
	return expr;
}

std::pair<bool, Stmt*> ASTWalker::handleStmtPre(Stmt* stmt)
{
	return { true, stmt };
}

Stmt* ASTWalker::handleStmtPost(Stmt* stmt)
{
	return stmt;
}

std::pair<bool, Decl*> ASTWalker::handleDeclPre(Decl* decl)
{
	return { true, decl };
}

Decl* ASTWalker::handleDeclPost(Decl* decl)
{
	return decl;
}

std::pair<bool, Type*> ASTWalker::handleTypePre(Type* type)
{
	return { true, type };
}

Type* ASTWalker::handleTypePost(Type* type)
{
	return type;
}
