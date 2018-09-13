////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : SemaExprs.cpp										
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
//	This file implements Sema methods related to Exprs
////------------------------------------------------------////

#include "Sema.hpp"
#include "Fox/AST/Expr.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/Common/LLVM.hpp"
#include "Fox/AST/ASTVisitor.hpp"

using namespace fox;

class ExprChecker : public ExprVisitor<ExprChecker, Sema::ExprResult>
{
	public:
		using ResultTy = Sema::ExprResult;

	private:

		template<typename ParentTy, typename SetterTy>
		ResultTy doIt(ParentTy* node, Expr* child, SetterTy setter)
		{
			// Visit
			auto result = visit(child);

			// Replace if needed
			if (result.hasReplacement())
				((*node).*setter)(result.getReplacement());

			return result;
		}

	public:
		Sema& sema;

		ExprChecker(Sema& inst) : sema(inst)
		{

		}

		// Each of theses method must (in that order)
		// 1. Call visit on each of it's children (using doIt)
		// 2. If every children's visit was successful
		//		call checkXXX on the argument and return that.
		//		Never use check(), always use the specialized version.

		ResultTy visitParensExpr(ParensExpr* node)
		{
			auto result = doIt(node, node->getExpr(), &ParensExpr::setExpr);
			if (result)
				return sema.checkParensExpr(node);
			return ResultTy::Failure();
		}

		ResultTy visitBinaryExpr(BinaryExpr*)
		{
			return ResultTy::Success();
		}

		ResultTy visitUnaryExpr(UnaryExpr*)
		{
			return ResultTy::Success();
		}

		ResultTy visitCastExpr(CastExpr*)
		{
			return ResultTy::Success();
		}

		ResultTy visitArrayAccessExpr(ArrayAccessExpr*)
		{
			return ResultTy::Success();
		}

		ResultTy visitCharLiteralExpr(CharLiteralExpr*)
		{
			return ResultTy::Success();
		}

		ResultTy visitBoolLiteralExpr(BoolLiteralExpr*)
		{
			return ResultTy::Success();
		}

		ResultTy visitIntegerLiteralExpr(IntegerLiteralExpr*)
		{
			return ResultTy::Success();
		}

		ResultTy visitFloatLiteralExpr(FloatLiteralExpr*)
		{
			return ResultTy::Success();
		}

		ResultTy visitStringLiteralExpr(StringLiteralExpr*)
		{
			return ResultTy::Success();
		}

		ResultTy visitArrayLiteralExpr(ArrayLiteralExpr*)
		{
			return ResultTy::Success();
		}

		ResultTy visitDeclRefExpr(DeclRefExpr*)
		{
			return ResultTy::Success();
		}

		ResultTy visitMemberOfExpr(MemberOfExpr*)
		{
			return ResultTy::Success();
		}

		ResultTy visitFunctionCallExpr(FunctionCallExpr*)
		{
			return ResultTy::Success();
		}
};

Sema::Sema::ExprResult Sema::checkExpr(Expr* node)
{
	ExprChecker checker(*this);
	checker.visit(node);
	return ExprResult::Success();
}

Sema::ExprResult Sema::checkParensExpr(ParensExpr*)
{
	return ExprResult::Success();
}

Sema::ExprResult Sema::checkBinaryExpr(BinaryExpr*)
{
	return ExprResult::Success();
}

Sema::ExprResult Sema::checkUnaryExpr(UnaryExpr*)
{
	return ExprResult::Success();
}

Sema::ExprResult Sema::checkCastExpr(CastExpr*)
{
	return ExprResult::Success();
}

Sema::ExprResult Sema::checkArrayAccessExpr(ArrayAccessExpr*)
{
	return ExprResult::Success();
}

Sema::ExprResult Sema::checkCharLiteralExpr(CharLiteralExpr*)
{
	return ExprResult::Success();
}

Sema::ExprResult Sema::checkBoolLiteralExpr(BoolLiteralExpr*)
{
	return ExprResult::Success();
}

Sema::ExprResult Sema::checkIntegerLiteralExpr(IntegerLiteralExpr*)
{
	return ExprResult::Success();
}

Sema::ExprResult Sema::checkFloatLiteralExpr(FloatLiteralExpr*)
{
	return ExprResult::Success();
}

Sema::ExprResult Sema::checkStringLiteralExpr(StringLiteralExpr*)
{
	return ExprResult::Success();
}

Sema::ExprResult Sema::checkArrayLiteralExpr(ArrayLiteralExpr*)
{
	return ExprResult::Success();
}

Sema::ExprResult Sema::checkDeclRefExpr(DeclRefExpr*)
{
	return ExprResult::Success();
}

Sema::ExprResult Sema::checkMemberOfExpr(MemberOfExpr*)
{
	return ExprResult::Success();
}

Sema::ExprResult Sema::checkFunctionCallExpr(FunctionCallExpr*)
{
	return ExprResult::Success();
}
