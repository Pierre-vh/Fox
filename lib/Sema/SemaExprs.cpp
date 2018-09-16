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
#include "Fox/AST/ASTVisitor.hpp"

#include <utility>
#include <iostream>

using namespace fox;

class Sema::ExprChecker : public ExprVisitor<ExprChecker, Sema::ExprResult>
{
	public:
		using ResultTy = Sema::ExprResult;

	private:

		// DoIt: calls Visit on a node and, after the visit, if the node
		// has a replacement available, replaces it using the setter given as
		// argument.
		template<typename DerivedTy, typename ... Args>
		bool doIt(DerivedTy* node, Expr* child, void (DerivedTy::*setter)(Expr*, Args...), Args... args)
		{
			// Visit
			auto result = visit(child);

			// Replace if needed
			if (result.hasReplacement())
				((*node).*setter)(result.getReplacement(), std::forward<Args>(args)...);

			return result.wasSuccessful();
		}

	public:
		Sema & sema;

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
			if (doIt(node, node->getExpr(), &ParensExpr::setExpr))
				return sema.checkParensExpr(node);
			return ResultTy::Failure();
		}

		ResultTy visitBinaryExpr(BinaryExpr* node)
		{
			bool lhs = doIt(node, node->getLHS(), &BinaryExpr::setLHS);
			bool rhs = doIt(node, node->getRHS(), &BinaryExpr::setRHS);

			if (lhs && rhs)
				return sema.checkBinaryExpr(node);
			return ResultTy::Failure();
		}

		ResultTy visitUnaryExpr(UnaryExpr* node)
		{
			if (doIt(node, node->getExpr(), &UnaryExpr::setExpr))
				return sema.checkUnaryExpr(node);
			return ResultTy::Failure();
		}

		ResultTy visitCastExpr(CastExpr* node)
		{
			bool expr = doIt(node, node->getExpr(), &CastExpr::setExpr);

			if (expr)
				return sema.checkCastExpr(node);
			return ResultTy::Failure();
		}

		ResultTy visitArrayAccessExpr(ArrayAccessExpr* node)
		{
			bool idx = doIt(node, node->getIdxExpr(), &ArrayAccessExpr::setIdxExpr);
			bool base = doIt(node, node->getExpr(), &ArrayAccessExpr::setExpr);

			if (idx && base)
				return sema.checkArrayAccessExpr(node);
			return ResultTy::Failure();
		}

		ResultTy visitCharLiteralExpr(CharLiteralExpr* node)
		{
			return sema.checkCharLiteralExpr(node);
		}

		ResultTy visitBoolLiteralExpr(BoolLiteralExpr* node)
		{
			return sema.checkBoolLiteralExpr(node);
		}

		ResultTy visitIntegerLiteralExpr(IntegerLiteralExpr* node)
		{
			return sema.checkIntegerLiteralExpr(node);
		}

		ResultTy visitFloatLiteralExpr(FloatLiteralExpr* node)
		{
			return sema.checkFloatLiteralExpr(node);
		}

		ResultTy visitStringLiteralExpr(StringLiteralExpr* node)
		{
			return sema.checkStringLiteralExpr(node);
		}

		ResultTy visitArrayLiteralExpr(ArrayLiteralExpr* node)
		{
			bool flag = true;
			for (auto it = node->exprs_begin(), end = node->exprs_end(); it != end; it++)
				flag = flag && doIt(node, (*it), &ArrayLiteralExpr::setExpr, it);

			if (flag)
				return sema.checkArrayLiteralExpr(node);
			return ResultTy::Failure();
		}

		ResultTy visitDeclRefExpr(DeclRefExpr* node)
		{
			return sema.checkDeclRefExpr(node);
		}

		ResultTy visitMemberOfExpr(MemberOfExpr* node)
		{
			if (doIt(node, node->getExpr(), &MemberOfExpr::setExpr))
				return sema.checkMemberOfExpr(node);
			return ResultTy::Failure();
		}

		ResultTy visitFunctionCallExpr(FunctionCallExpr* node)
		{
			bool flag = doIt(node, node->getCallee(), &FunctionCallExpr::setCallee);

			for (auto it = node->args_begin(), end = node->args_end(); it != end; it++)
				flag = flag && doIt(node, (*it), &FunctionCallExpr::setArg, it);

			if (flag)
				return sema.checkFunctionCallExpr(node);
			return ResultTy::Failure();
		}
};

Sema::Sema::ExprResult Sema::checkExpr(Expr* node)
{
	ExprChecker checker(*this);
	return checker.visit(node);
}

Sema::ExprResult Sema::checkParensExpr(ParensExpr*)
{
	std::cout << "ParensExpr" << std::endl;
	return ExprResult::Success();
}

Sema::ExprResult Sema::checkBinaryExpr(BinaryExpr*)
{
	std::cout << "BinaryExpr" << std::endl;
	return ExprResult::Success();
}

Sema::ExprResult Sema::checkUnaryExpr(UnaryExpr*)
{
	std::cout << "UnaryExpr" << std::endl;
	return ExprResult::Success();
}

Sema::ExprResult Sema::checkCastExpr(CastExpr*)
{
	std::cout << "CastExpr" << std::endl;
	return ExprResult::Success();
}

Sema::ExprResult Sema::checkArrayAccessExpr(ArrayAccessExpr*)
{
	std::cout << "ArrayAccessExpr" << std::endl;
	return ExprResult::Success();
}

Sema::ExprResult Sema::checkCharLiteralExpr(CharLiteralExpr*)
{
	std::cout << "CharLiteralExpr" << std::endl;
	return ExprResult::Success();
}

Sema::ExprResult Sema::checkBoolLiteralExpr(BoolLiteralExpr*)
{
	std::cout << "BoolLiteralExpr" << std::endl;
	return ExprResult::Success();
}

Sema::ExprResult Sema::checkIntegerLiteralExpr(IntegerLiteralExpr*)
{
	std::cout << "IntegerLiteralExpr" << std::endl;
	return ExprResult::Success();
}

Sema::ExprResult Sema::checkFloatLiteralExpr(FloatLiteralExpr*)
{
	std::cout << "FloatLiteralExpr" << std::endl;
	return ExprResult::Success();
}

Sema::ExprResult Sema::checkStringLiteralExpr(StringLiteralExpr*)
{
	std::cout << "StringLiteralExpr" << std::endl;
	return ExprResult::Success();
}

Sema::ExprResult Sema::checkArrayLiteralExpr(ArrayLiteralExpr*)
{
	std::cout << "ArrayLiteralExpr" << std::endl;
	return ExprResult::Success();
}

Sema::ExprResult Sema::checkDeclRefExpr(DeclRefExpr*)
{
	std::cout << "DeclRefExpr" << std::endl;
	return ExprResult::Success();
}

Sema::ExprResult Sema::checkMemberOfExpr(MemberOfExpr*)
{
	std::cout << "MemberOfExpr" << std::endl;
	return ExprResult::Success();
}

Sema::ExprResult Sema::checkFunctionCallExpr(FunctionCallExpr*)
{
	std::cout << "FunctionCallExpr" << std::endl;
	return ExprResult::Success();
}
