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

Sema::Sema::SemaResult Sema::check(Expr*)
{
	return SemaResult::Success();
}

Sema::SemaResult Sema::checkParensExpr(ParensExpr*)
{
	return SemaResult::Success();
}

Sema::SemaResult Sema::checkBinaryExpr(BinaryExpr*)
{
	return SemaResult::Success();
}

Sema::SemaResult Sema::checkUnaryExpr(UnaryExpr*)
{
	return SemaResult::Success();
}

Sema::SemaResult Sema::checkCastExpr(CastExpr*)
{
	return SemaResult::Success();
}

Sema::SemaResult Sema::checkArrayAccessExpr(ArrayAccessExpr*)
{
	return SemaResult::Success();
}

Sema::SemaResult Sema::checkCharLiteralExpr(CharLiteralExpr*)
{
	return SemaResult::Success();
}

Sema::SemaResult Sema::checkBoolLiteralExpr(BoolLiteralExpr*)
{
	return SemaResult::Success();
}

Sema::SemaResult Sema::checkIntegerLiteralExpr(IntegerLiteralExpr*)
{
	return SemaResult::Success();
}

Sema::SemaResult Sema::checkFloatLiteralExpr(FloatLiteralExpr*)
{
	return SemaResult::Success();
}

Sema::SemaResult Sema::checkStringLiteralExpr(StringLiteralExpr*)
{
	return SemaResult::Success();
}

Sema::SemaResult Sema::checkArrayLiteralExpr(ArrayLiteralExpr*)
{
	return SemaResult::Success();
}

Sema::SemaResult Sema::checkDeclRefExpr(DeclRefExpr*)
{
	return SemaResult::Success();
}

Sema::SemaResult Sema::checkMemberOfExpr(MemberOfExpr*)
{
	return SemaResult::Success();
}

Sema::SemaResult Sema::checkFunctionCallExpr(FunctionCallExpr*)
{
	return SemaResult::Success();
}
