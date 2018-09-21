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
		public:
			Traverse(ASTWalker& walker):
				walker_(walker)
			{}

			Expr* visitParensExpr(ParensExpr* expr)
			{
				if (Expr* node = doIt(expr->getExpr()))
				{
					expr->setExpr(node);
					return expr;
				}
				return nullptr;
			}

		private:
			// doIt method for expression: handles call to the walker &
			// requests visitation of the children of a given node.
			Expr* doIt(Expr* expr)
			{
				// Let the walker handle the pre visitation stuff.
				auto rtr = walker_.handleExprPre(expr);

				// Return if we have a nullptr or if we're instructed
				// to not visit the children.
				if (!rtr.first || !rtr.second)
					return rtr.first;

				// visit the node's childre, and if the traversal wasn't aborted,
				// let the walker handle post visitation stuff.
				if (expr = visit(rtr.first))
					expr = walker_.handleExprPost(expr);

				return expr;
			}

			// doIt method for declarations: handles call to the walker &
			// requests visitation of the children of a given node.
			Decl* doIt(Decl* expr)
			{
				// Let the walker handle the pre visitation stuff.
				auto rtr = walker_.handleDeclPre(expr);

				// Return if we have a nullptr or if we're instructed
				// to not visit the children.
				if (!rtr.first || !rtr.second)
					return rtr.first;

				// visit the node's childre, and if the traversal wasn't aborted,
				// let the walker handle post visitation stuff.
				if (expr = visit(rtr.first))
					expr = walker_.handleDeclPost(expr);

				return expr;
			}

			// doIt method for statements: handles call to the walker &
			// requests visitation of the children of a given node.
			Stmt* doIt(Stmt* expr)
			{
				// Let the walker handle the pre visitation stuff.
				auto rtr = walker_.handleStmtPre(expr);

				// Return if we have a nullptr or if we're instructed
				// to not visit the children.
				if (!rtr.first || !rtr.second)
					return rtr.first;

				// visit the node's childre, and if the traversal wasn't aborted,
				// let the walker handle post visitation stuff.
				if (expr = visit(rtr.first))
					expr = walker_.handleStmtPost(expr);

				return expr;
			}

			// doIt method for types: handles call to the walker &
			// requests visitation of the children of a given node.
			Type* doIt(Type* expr)
			{
				// Let the walker handle the pre visitation stuff.
				auto rtr = walker_.handleTypePre(expr);

				// Return if we have a nullptr or if we're instructed
				// to not visit the children.
				if (!rtr.first || !rtr.second)
					return rtr.first;

				// visit the node's childre, and if the traversal wasn't aborted,
				// let the walker handle post visitation stuff.
				if (expr = visit(rtr.first))
					expr = walker_.handleTypePost(expr);

				return expr;
			}

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

std::pair<Expr*, bool> ASTWalker::handleExprPre(Expr* expr)
{
	return { expr, true };
}

Expr* ASTWalker::handleExprPost(Expr* expr)
{
	return expr;
}

std::pair<Stmt*, bool> ASTWalker::handleStmtPre(Stmt* stmt)
{
	return { stmt, true };
}

Stmt* ASTWalker::handleStmtPost(Stmt* stmt)
{
	return stmt;
}

std::pair<Decl*, bool> ASTWalker::handleDeclPre(Decl* decl)
{
	return { decl, true };
}

Decl* ASTWalker::handleDeclPost(Decl* decl)
{
	return decl;
}

std::pair<Type*, bool> ASTWalker::handleTypePre(Type* type)
{
	return { type, true };
}

Type* ASTWalker::handleTypePost(Type* type)
{
	return type;
}
