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
	class ExprChecker : public ExprVisitor<ExprChecker, void>
	{
		using Inherited = ExprVisitor<ExprChecker, void>;
		public:
			ExprChecker()
			{

			}

			void visit(Expr* expr)
			{
				Inherited::visit(expr);
				assert(expr->getType() && "Expression is not typed after checking");
			}

			void visitParensExpr(ParensExpr* expr)
			{
				expr->setType(expr->getType());
			}

			void visitBinaryExpr(BinaryExpr* expr)
			{
				// Handle concatenation
			}

			void visitCastExpr(CastExpr*){}
			void visitUnaryExpr(UnaryExpr*)
			{
			
			}

			void visitArrayAccessExpr(ArrayAccessExpr*){}
			void visitMemberOfExpr(MemberOfExpr*){}
			void visitDeclRefExpr(DeclRefExpr*){}
			void visitFunctionCallExpr(FunctionCallExpr*){}

			void visitCharLiteralExpr(CharLiteralExpr*){}
			void visitIntegerLiteralExpr(IntegerLiteralExpr*){}
			void visitFloatLiteralExpr(FloatLiteralExpr*){}
			void visitBooleanLiteralExpr(BoolLiteralExpr*){}
			void visitStringLiteralExpr(StringLiteralExpr*){}
			void visitArrayLiteralExpr(ArrayLiteralExpr*){}
	};


	class ExprWalker : public ASTWalker
	{
		public:
			virtual std::pair<Expr*, bool> handleExprPre(Expr* expr)
			{
				// Not needed since we won't do preorder visitation
				return { expr, true }; // Important for postorder visitation to be done
			}

			virtual Expr* handleExprPost(Expr* expr)
			{

			}

			virtual std::pair<Stmt*, bool> handleStmtPre(Stmt* stmt)
			{
				fox_unreachable("Illegal node kind");
			}

			virtual Stmt* handleStmtPost(Stmt* stmt)
			{
				fox_unreachable("Illegal node kind");
			}

			virtual std::pair<Decl*, bool> handleDeclPre(Decl* decl)
			{
				fox_unreachable("Illegal node kind");
			}

			virtual Decl* handleDeclPost(Decl* decl)
			{
				fox_unreachable("Illegal node kind");
			}
		private:

	};
} // End anonymous namespace