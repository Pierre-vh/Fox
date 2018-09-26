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
	class ExprChecker : public ExprVisitor<ExprChecker, Expr*>, public ASTWalker
	{
		using Inherited = ExprVisitor<ExprChecker, Expr*>;
		Sema& sema_;
		public:
			ExprChecker(Sema& sema): 
				sema_(sema)
			{
				
			}

			// Sets the expr's type to ErrorType
			void setErrorType(Expr* expr)
			{
				expr->setType(sema_.getASTContext().getErrorType());
			}

			// Returns the ASTContext
			ASTContext& getCtxt()
			{
				return sema_.getASTContext();
			}

			// Returns the DiagnosticEngine
			DiagnosticEngine& getDiags()
			{
				return sema_.getDiagnosticEngine();
			}

			Sema& getSema()
			{
				return sema_;
			}

			virtual std::pair<Expr*, bool> handleExprPre(Expr* expr)
			{
				// Not needed since we won't do preorder visitation
				return { expr, true }; // Important for postorder visitation to be done
			}

			virtual Expr* handleExprPost(Expr* expr)
			{
				return visit(expr);
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
				assert(expr && "Expression is null");
				assert(expr->getType() && "Expression is not typed after checking");
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
				// WIP: Check if cast is okay
				TypeBase* exprTy = expr->getExpr()->getType();
				TypeBase* castGoal = expr->getCastGoal();

				if (!getSema().unify(exprTy, castGoal))
				{
					// Add special user friendly type dump in sema instead of the getString method
					getDiags()
						.report(DiagID::sema_invalid_cast, expr->getTypeRange())
							.addArg(exprTy->getString())
							.addArg(castGoal->getString())
							.setExtraRange(expr->getExpr()->getRange());
					expr->setType(getCtxt().getErrorType());
					// Propagate the error type to the expr->type to avoid error
					// flooding.
				}
				else
					expr->setType(castGoal);

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
				expr->setType(getCtxt().getCharType());
				return expr;
			}

			Expr* visitIntegerLiteralExpr(IntegerLiteralExpr* expr)
			{
				expr->setType(getCtxt().getIntType());
				return expr;
			}

			Expr* visitFloatLiteralExpr(FloatLiteralExpr* expr)
			{
				expr->setType(getCtxt().getFloatType());
				return expr;
			}

			Expr* visitBoolLiteralExpr(BoolLiteralExpr* expr)
			{
				expr->setType(getCtxt().getBoolType());
				return expr;
			}

			Expr* visitStringLiteralExpr(StringLiteralExpr* expr)
			{
				expr->setType(getCtxt().getStringType());
				return expr;
			}

			// Array literals
			Expr* visitArrayLiteralExpr(ArrayLiteralExpr* expr)
			{
				if (auto size = expr->getSize())
				{
					TypeBase* proposed = nullptr;
					// Deduce the type by starting from the first type
					// and by upranking it if needed.
					for (auto& elem : expr->getExprs())
					{
						TypeBase* elemTy = elem->getType();

						// First loop, set proposed & continue.
						if (!proposed)
						{
							proposed = elemTy;
							continue;
						}

						// Handle error elem type.
						if (isa<ErrorType>(elemTy))
						{
							// Stop progression & break
							proposed = nullptr;
							break;
						}

						// Unify the proposed type with the elemTy
						// We do elemTy = proposed instead of the opposite
						// because we want to elemTy's sub to be set to proposed
						// if it's a Sematype awaiting inferrence.
						if (!getSema().unify(elemTy, proposed))
						{
							// Failed to unify: incompatible types
							// TODO: Do a more precise diagnostic as this one might be unhelpful
							// in some cases.
							getDiags().report(DiagID::sema_arraylit_hetero, expr->getRange());
							//std::cout << "Array was thought to be of type " << proposed->getString() << " but found " << elemTy->getString() << std::endl;
							proposed = nullptr;
							break;
						}

						// Lastly, uprank if needed.
						if (TypeBase* highestRanking = Sema::getHighestRankingType(elemTy, proposed))
							proposed = highestRanking;
					}

					// Apply.
					if (proposed)
					{
						// TypeBase is an array of the proposed type.
						expr->setType(getCtxt().getArrayTypeForType(proposed));
					}
					else
						setErrorType(expr); // Failed to typecheck.
				}
				else
				{
					// Let type inference do it's magic by requiring a arraytype of any type.
					SemaType* fresh = getCtxt().createSemaType();
					ArrayType* ty = getCtxt().getArrayTypeForType(fresh);
					expr->setType(ty);
				}
				return expr;
			}
	};

	// ExprFinalizer, which rebuilds types to remove
	// SemaTypes.
	// Visit methods return pointers to TypeBase. They return nullptr
	// if the finalization failed for this expr.
	// It's still a primitive, test version for now.
	class ExprFinalizer : public TypeVisitor<ExprFinalizer, TypeBase*>, public ASTWalker
	{
		ASTContext& ctxt_;
		DiagnosticEngine& diags_;

		public:
			ExprFinalizer(ASTContext& ctxt, DiagnosticEngine& diags) :
				ctxt_(ctxt), diags_(diags)
			{

			}

			Expr* handleExprPost(Expr* expr)
			{
				TypeBase* type = expr->getType();
				assert(type && "Untyped expr");

				// Visit the type
				type = visit(type);
				// If the type is nullptr, this inference failed
				// because of a lack of substitution somewhere.
				// Set the type to ErrorType, diagnose it and move on.
				if (!type)
				{
					diags_.report(DiagID::sema_failed_infer, expr->getRange());
					type = ctxt_.getErrorType();
				}
				expr->setType(type);
				return expr;
			}

			TypeBase* visitPrimitiveType(PrimitiveType* type)
			{
				return type;
			}

			TypeBase* visitArrayType(ArrayType* type)
			{
				if (TypeBase* elem = visit(type->getElementType()))
				{
					// Rebuild if needed
					if(elem != type->getElementType())
						return ctxt_.getArrayTypeForType(elem);
					return type;
				}
				return nullptr;
			}

			TypeBase* visitLValueType(LValueType* type)
			{
				if (TypeBase* elem = visit(type->getType()))
				{
					if(elem != type->getType())
						return ctxt_.getLValueTypeForType(elem);
					return type;
				}
				return nullptr;
			}

			TypeBase* visitSemaType(SemaType* type)
			{
				if (TypeBase* sub = type->getSubstitution())
					return visit(sub);
				return nullptr;
			}

			TypeBase* visitErrorType(ErrorType* type)
			{
				// Error should have been handled already, we won't emit
				// more.
				return type;
			}
	};
} // End anonymous namespace

Expr* Sema::typecheckExpr(Expr* expr)
{
	expr = ExprChecker(*this).walk(expr);
	//expr = ExprFinalizer(ctxt_, diags_).walk(expr);
	return expr;
}