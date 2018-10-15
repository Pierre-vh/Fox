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
	//
	// Every visitation method return a pointer to an Expr*, which is the current expr
	// OR the expr that should take it's place. This can NEVER be null.
	class ExprChecker : public ExprVisitor<ExprChecker, Expr*>, public ASTWalker
	{
		using Inherited = ExprVisitor<ExprChecker, Expr*>;
		Sema& sema_;
		public:
			ExprChecker(Sema& sema): 
				sema_(sema)
			{
				
			}

			// Returns the Int type if type is a boolean, or
			// return it's argument otherwise.
			// The pointer must not be null an point to an integral type.
			TypeBase* uprankIfBoolean(PrimitiveType* type)
			{
				assert(type && "Pointer must not be null");
				assert(Sema::isIntegral(type) && "Type must be integral");

				if (type->getPrimitiveKind() == PrimitiveType::Kind::BoolTy)
					return PrimitiveType::getInt(getCtxt());
				return type;
			}

			// Sets the expr's type to ErrorType
			ErrorType* getErrorType()
			{
				return ErrorType::get(getCtxt());
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
				Type& exprTy = expr->getExpr()->getType();
				TypeLoc& castGoal = expr->getCastTypeLoc();

				if (!getSema().unify(exprTy, castGoal))
				{
					if(!exprTy.is<ErrorType>() && !castGoal.is<ErrorType>())
						getDiags()
							.report(DiagID::sema_invalid_cast, castGoal.getRange())
								.addArg(exprTy->toString())
								.addArg(castGoal->toString())
								.setExtraRange(expr->getExpr()->getRange());
					expr->setType(getErrorType());
					// Propagate the error type to the expr->type to avoid error
					// flooding.
				}
				else
					expr->setType(castGoal.withoutLoc());
				// TODO: Allow cast to string? If I do allow that,
				// I should mark the castExpr as being a "Stringifying" one,
				// so IR gen can emit the appropriate instr.
				return expr;
			}

			Expr* visitUnaryExpr(UnaryExpr* expr)
			{
				Expr* child = expr->getExpr();
				Type childTy = child->getType();

				// For any unary operators, we only allow integral types,
				// so check that first.
				if (!Sema::isIntegral(childTy))
				{
					// Not an integral type -> error.
					expr->setType(getErrorType());
					// Emit diag iff childTy isn't a ErrorType too
					if (!childTy.is<ErrorType>())
					{
						getDiags()
							.report(DiagID::sema_unaryop_bad_child_type, expr->getOpRange())
							.setExtraRange(child->getRange()) // Use the child's range as the extra range.
							.addArg(expr->getOpSign()) // %0 is the operator's sign as text
							.addArg(childTy->toString()); // %1 is the type of the child
					}
					return expr;
				}
				
				PrimitiveType* primChildTy = dyn_cast<PrimitiveType>(childTy->ignoreLValue());
				assert(primChildTy && "isIntegral returned true but the type isn't a PrimitiveType?");

				using OP = UnaryExpr::OpKind;
				switch (expr->getOp())
				{
					// Logical NOT operator : '!'
					case OP::LNot:
						// Always boolean
						expr->setType(PrimitiveType::getBool(getCtxt()));
						break;
					// Unary Plus '+' and Minus '-'
					case OP::Minus:
					case OP::Plus:
						// Always int or float, never bool, so uprank
						// if boolean.
						expr->setType(uprankIfBoolean(primChildTy));
						break;
					case OP::Invalid:
						fox_unreachable("Invalid Unary Operator should not exist past parsing");
					default:
						fox_unreachable("All cases handled");
				}

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
				// Will be left unimplemented for now
				return expr;
			}

			Expr* visitDeclRefExpr(DeclRefExpr* expr)
			{
				// Will be left unimplemented for now
				return expr;
			}

			Expr* visitFunctionCallExpr(FunctionCallExpr* expr)
			{
				// Will be left unimplemented for now
				return expr;
			}
			

			// Trivial literals: the expr's type is simply the corresponding
			// type. Int for a Int literal, etc.
			Expr* visitCharLiteralExpr(CharLiteralExpr* expr)
			{
				expr->setType(PrimitiveType::getChar(getCtxt()));
				return expr;
			}

			Expr* visitIntegerLiteralExpr(IntegerLiteralExpr* expr)
			{
				expr->setType(PrimitiveType::getInt(getCtxt()));
				return expr;
			}

			Expr* visitFloatLiteralExpr(FloatLiteralExpr* expr)
			{
				expr->setType(PrimitiveType::getFloat(getCtxt()));
				return expr;
			}

			Expr* visitBoolLiteralExpr(BoolLiteralExpr* expr)
			{
				expr->setType(PrimitiveType::getBool(getCtxt()));
				return expr;
			}

			Expr* visitStringLiteralExpr(StringLiteralExpr* expr)
			{
				expr->setType(PrimitiveType::getString(getCtxt()));
				return expr;
			}

			ConstrainedType* createConstrainedTypeForEmptyArrayLiteral()
			{
				auto* cs = ConstrainedType::create(getCtxt());
				cs->addConstraint(Constraint::createArrayCS(getCtxt()));
				return cs;
			}

			// Array literals
			// To deduce the type of an Array literal:
			// if size > 0
			//		Create a "proposed" type. For the first
			//		iteration of the loop, set proposed to elemTy and continue,
			//		After, unify the elemTy with the proposed, if rank(deferIf(elemTy)) > rank(deferIf(proposed)) -> proposed = elemTy.
			// else
			//		Type needs inference
			Expr* visitArrayLiteralExpr(ArrayLiteralExpr* expr)
			{
				// Macro that unwraps a type if it's a ConstrainedType
				// with a valid substitution. Else it simply returns it's argument.
				static auto defer_if_possible = [](Type t) {
					if (auto* ptr = t.getAs<ConstrainedType>())
					{
						// if the type has a substitution, return it, else
						// just return the argument.
						if(auto* sub = ptr->getSubstitution())
							return Type(sub);
					}
					return t;
				};

				if (auto size = expr->getSize())
				{
					Type proposed = nullptr;
					// Deduce the type by starting from the first type
					// and by upranking it if needed.
					for (auto& elem : expr->getExprs())
					{
						Type& elemTy = elem->getType();

						// Handle error elem type.
						if (elemTy.is<ErrorType>())
						{
							// Stop progression & break
							proposed = nullptr;
							break;
						}

						// First loop, set proposed & continue.
						if (!proposed)
						{
							proposed = elemTy;
							continue;
						}

						// If both elemTy and proposed are ConstrainedTypes with
						// no substitution, what should I do?

						// Attempt to unify elemTy with the proposed type.
						if (!getSema().unify(elemTy, proposed))
						{
							// Failed to unify: incompatible types
							if(!elemTy.is<ErrorType>() && !proposed.is<ErrorType>())
								getDiags().report(DiagID::sema_arraylit_hetero, expr->getRange());
							std::cout << "Array was thought to be of type " << proposed->toString() << " but found " << elemTy->toString() << std::endl;
							proposed = nullptr;
							break;
						}

						// Get the highest ranking type of both types
						Type highestRanking =
							Sema::getHighestRankingType(
								defer_if_possible(elemTy),
								defer_if_possible(proposed),
								/*ignoreLValues*/ true,
								/*unwrapTypes*/ true);
						if (highestRanking)
						{
							std::cout << "Replacing " << proposed->toDebugString() << "(" << proposed.getPtr() << ")"
								<< " with " << highestRanking->toDebugString() << "(" << highestRanking.getPtr() << ")" << "\n";
							proposed = highestRanking;
						}
					}

					// Apply.
					if (proposed)
					{
						// TypeBase is an array of the proposed type.
						expr->setType(ArrayType::get(getCtxt(), proposed.getPtr()));
					}
					else
						expr->setType(getErrorType());
				}
				else
				{
					// Let type inference do it's magic 
					expr->setType(createConstrainedTypeForEmptyArrayLiteral());
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
				Type type = expr->getType().getPtr();
				assert(!type.isNull() && "Untyped expr");

				// Visit the type
				type = visit(type.getPtr());
				// If the type is nullptr, this inference failed
				// because of a lack of substitution somewhere.
				// Set the type to ErrorType, diagnose it and move on.
				if (!type)
				{
					diags_.report(DiagID::sema_failed_infer, expr->getRange());
					type = ErrorType::get(ctxt_);
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
					if (elem != type->getElementType())
						return ArrayType::get(ctxt_, elem);
					return type;
				}
				return nullptr;
			}

			TypeBase* visitLValueType(LValueType* type)
			{
				if (TypeBase* elem = visit(type->getType()))
				{
					if (elem != type->getType())
						return LValueType::get(ctxt_, elem);
					return type;
				}
				return nullptr;
			}

			TypeBase* visitConstrainedType(ConstrainedType* type)
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
	expr = ExprFinalizer(ctxt_, diags_).walk(expr);
	return expr;
}