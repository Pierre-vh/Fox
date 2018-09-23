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

Sema::Sema::ExprResult Sema::checkExpr(Expr*)
{
	// todo
	return ExprResult::Failure();
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
