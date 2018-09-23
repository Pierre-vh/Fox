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

			// Exprs

			Expr* visitParensExpr(ParensExpr* expr)
			{
				if (Expr* node = doIt(expr->getExpr()))
				{
					expr->setExpr(node);
					return expr;
				}
				return nullptr;
			}

			Expr* visitBinaryExpr(BinaryExpr* expr)
			{
				Expr* lhs = doIt(expr->getLHS());
				if(!lhs) return nullptr;
				expr->setLHS(lhs);

				Expr* rhs = doIt(expr->getRHS());
				if(!rhs) return nullptr;
				expr->setRHS(rhs);

				return expr;
			}

			Expr* visitUnaryExpr(UnaryExpr* expr)
			{
				if (Expr* child = doIt(expr->getExpr()))
				{
					expr->setExpr(child);
					return expr;
				}
				return nullptr;
			}

			Expr* visitCastExpr(CastExpr* expr)
			{
				if (Expr* child = doIt(expr->getExpr()))
				{
					expr->setExpr(child);
					return expr;
				}
				return nullptr;
			}

			Expr* visitArrayAccessExpr(ArrayAccessExpr* expr)
			{
				Expr* base = doIt(expr->getExpr()); 
				if(!base) return nullptr;
				expr->setExpr(base);

				Expr* idx = doIt(expr->getIdxExpr());
				if(!idx) return nullptr;
				expr->setIdxExpr(idx);

				return expr;
			}

			#define TRIVIAL_EXPR_VISIT(NODE) Expr* visit##NODE(NODE* expr) { return expr; }
			TRIVIAL_EXPR_VISIT(CharLiteralExpr)
			TRIVIAL_EXPR_VISIT(BoolLiteralExpr)
			TRIVIAL_EXPR_VISIT(IntegerLiteralExpr)
			TRIVIAL_EXPR_VISIT(StringLiteralExpr)
			TRIVIAL_EXPR_VISIT(FloatLiteralExpr)
			TRIVIAL_EXPR_VISIT(DeclRefExpr)
			#undef TRIVIAL_EXPR_VISIT
			
			Expr* visitArrayLiteralExpr(ArrayLiteralExpr* expr)
			{
				for (auto it = expr->exprs_begin(), end = expr->exprs_end(); it != end; it++)
				{
					if (Expr* node = doIt(*it))
						expr->setExpr(node, it);
					else return nullptr;
				}

				return expr;
			}

			Expr* visitMemberOfExpr(MemberOfExpr* expr)
			{
				if (Expr* child = doIt(expr->getExpr()))
				{
					expr->setExpr(child);
					return expr;
				}
				return nullptr;
			}

			Expr* visitFunctionCallExpr(FunctionCallExpr* expr)
			{
				Expr* callee = doIt(expr->getCallee());
				if(!callee) return nullptr;
				expr->setCallee(callee);

				for (auto it = expr->args_begin(), end = expr->args_end(); it != end; it++)
				{
					if (Expr* node = doIt(*it))
						expr->setArg(node, it);
					else return nullptr;
				}

				return expr;
			}


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
	return Traverse(*this).doIt(expr);
}

Decl* ASTWalker::walk(Decl* decl)
{
	return Traverse(*this).doIt(decl);
}

Stmt* ASTWalker::walk(Stmt* stmt)
{
	return Traverse(*this).doIt(stmt);
}

Type* ASTWalker::walk(Type* type)
{
	return Traverse(*this).doIt(type);
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
