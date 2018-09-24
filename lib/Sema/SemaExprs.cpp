////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : SemaExprs.cpp										
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
//	This file implements Sema methods related to Exprs
////------------------------------------------------------////

#include "Fox/Sema/Sema.hpp"
#include "Fox/AST/Expr.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/Common/LLVM.hpp"
#include "Fox/AST/ASTWalker.hpp"
#include "Fox/AST/ASTVisitor.hpp"

#include <utility>
#include <iostream>

using namespace fox;

namespace
{
	// Expression checker: Classic visitor, the visitXXX functions
	// all check a single node. They do not orchestrate visitation of
	// the children, because that is done in the ASTWalker

	// TODO:
	// Return Expr* or nothing? How to coordinate replacements?
	// Good idea would be to displace most of the logic in Sema!
	// e.g; visitBinaryExpr would call Sema::checkArithmeticAdditiveOperation(expr) if it
	// notices that 2 exprs are arithemtic.
	class ExprChecker : public ExprVisitor<ExprChecker, Expr*>, public ASTWalker
	{
		using Inherited = ExprVisitor<ExprChecker, Expr*>;
		ASTContext& ctxt_;
		Sema& sema_;
		public:
			ExprChecker(Sema& sema, ASTContext& ctxt): 
				ctxt_(ctxt), sema_(sema)
			{

			}

			virtual std::pair<Expr*, bool> handleExprPre(Expr* expr)
			{
				// Not needed since we won't do preorder visitation
				return { expr, true }; // Important for postorder visitation to be done
			}

			virtual Expr* handleExprPost(Expr* expr)
			{
				visit(expr);
			}

			virtual std::pair<Stmt*, bool> handleStmtPre(Stmt*)
			{
				fox_unreachable("Illegal node kind");
			}

			virtual Stmt* handleStmtPost(Stmt*)
			{
				fox_unreachable("Illegal node kind");
			}

			virtual std::pair<Decl*, bool> handleDeclPre(Decl*)
			{
				fox_unreachable("Illegal node kind");
			}

			virtual Decl* handleDeclPost(Decl*)
			{
				fox_unreachable("Illegal node kind");
			}

			Expr* visit(Expr* expr)
			{
				expr = Inherited::visit(expr);
				assert(expr && expr->getType() && "Expression is not typed after checking");
				return expr;
			}

			// Check methods

			Expr* visitParensExpr(ParensExpr* expr)
			{
				// A ParensExpr's type is simply it's child's
				expr->setType(expr->getType());
				// this node kind will be deleted later
				return expr;
			}

			Expr* visitBinaryExpr(BinaryExpr* expr)
			{
				// Handle arithmetic & text addition
				// Disallow array operation unless *
				return expr;
			}

			Expr* visitCastExpr(CastExpr* expr)
			{
				// Check if we can cast to that, castgoal must be
				// of the same family OR string.
				return expr;
			}

			Expr* visitUnaryExpr(UnaryExpr* expr)
			{
				// Check that the type's arithmetic.
				// For ! the return type is a bool, for everything else
				// it's int OR float if children is a float.
				return expr;
			}

			Expr* visitArrayAccessExpr(ArrayAccessExpr* expr)
			{
				// Check that base is of ArrayType and idx expr
				// is arithmetic and not float
				return expr;
			}

			Expr* visitMemberOfExpr(MemberOfExpr* expr)
			{
				// Unimplemented for now
				return expr;
			}

			Expr* visitDeclRefExpr(DeclRefExpr* expr)
			{
				// Unimplemented for now
				return expr;
			}

			Expr* visitFunctionCallExpr(FunctionCallExpr* expr)
			{
				// Unimplemented for now, but :
					// check that callee is a FunctionType or OverloadType
				return expr;
			}
			

			// Trivial literals: the expr's type is simply the corresponding
			// type. Int for a Int literal, etc.
			Expr* visitCharLiteralExpr(CharLiteralExpr* expr)
			{
				expr->setType(ctxt_.getCharType());
				return expr;
			}

			Expr* visitIntegerLiteralExpr(IntegerLiteralExpr* expr)
			{
				expr->setType(ctxt_.getIntType());
				return expr;
			}

			Expr* visitFloatLiteralExpr(FloatLiteralExpr* expr)
			{
				expr->setType(ctxt_.getFloatType());
				return expr;
			}

			Expr* visitBooleanLiteralExpr(BoolLiteralExpr* expr)
			{
				expr->setType(ctxt_.getBoolType());
				return expr;
			}

			Expr* visitStringLiteralExpr(StringLiteralExpr* expr)
			{
				expr->setType(ctxt_.getStringType());
				return expr;
			}

			// Array literals
			Expr* visitArrayLiteralExpr(ArrayLiteralExpr* expr)
			{
				if (auto size = expr->getSize())
				{
					Type* type = nullptr;
					for (auto& elem : expr->getExprs())
					{
						if (type)
						{
							// If they're of the same subtype, this function will return which one
							// of the 2 type is the highest ranked one. Else it returns nullptr.
							if (Type* tmp = sema_.getHighestRankingType(type, elem->getType()))
								type = tmp;
							else
							{
								type = nullptr;
								// TODO
									// Not the same subtype, emit a diagnostic and abort
									// Diagnostic should be about the whole array, like "cannot deduce array literal type"
							}
						}
						// First element, fine.
						else
							type = elem->getType();
					}

					// Apply.
					if (type)
						expr->setType(type);
					else
						expr->setType(ctxt_.getErrorType());

					return expr;
				}
				else
				{
					// Let type inference do it's magic by requiring a arraytype of any type.
					SemaType* fresh = ctxt_.createSemaType();
					ArrayType* ty = ctxt_.getArrayTypeForType(fresh);
					expr->setType(ty);
				}
				return expr;
			}
	};
} // End anonymous namespace