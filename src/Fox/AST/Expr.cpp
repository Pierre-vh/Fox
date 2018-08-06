////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Expr.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Expr.hpp"
#include "Identifiers.hpp"
#include "ASTContext.hpp"
#include "Fox/Common/Errors.hpp"
#include <map>
#include <sstream> 

using namespace fox;

// Expr
Expr::Expr(ExprKind kind, const SourceLoc& begLoc, const SourceLoc& endLoc) : kind_(kind), range_(begLoc, endLoc)
{

}

ExprKind Expr::getKind() const
{
	return kind_;
}

SourceRange Expr::getRange() const
{
	return range_;
}

SourceLoc Expr::getBegLoc() const
{
	return range_.getBegin();
}

SourceLoc Expr::getEndLoc() const
{
	return range_.getEnd();
}

void* Expr::operator new(std::size_t sz, ASTContext& ctxt, std::uint8_t align)
{
	return ctxt.getAllocator().allocate(sz, align);
}


// Literals : Char literals
CharLiteralExpr::CharLiteralExpr() : CharLiteralExpr(0, SourceLoc(), SourceLoc())
{

}

CharLiteralExpr::CharLiteralExpr(CharType val, const SourceLoc& begLoc, const SourceLoc& endLoc) 
	: val_(val), Expr(ExprKind::CharLiteralExpr,begLoc,endLoc)
{

}

CharType CharLiteralExpr::getVal() const
{
	return val_;
}

void CharLiteralExpr::setVal(CharType val)
{
	val_ = val;
}

// Literals : Integer literals
IntegerLiteralExpr::IntegerLiteralExpr() : IntegerLiteralExpr(0,SourceLoc(),SourceLoc())
{

}

IntegerLiteralExpr::IntegerLiteralExpr(IntType val, const SourceLoc& begLoc, const SourceLoc& endLoc)
	: val_(val), Expr(ExprKind::IntegerLiteralExpr,begLoc,endLoc)
{

}

IntType IntegerLiteralExpr::getVal() const
{
	return val_;
}

void IntegerLiteralExpr::setVal(IntType val)
{
	val_ = val;
}

// Literals : Float literals
FloatLiteralExpr::FloatLiteralExpr() : FloatLiteralExpr(0,SourceLoc(),SourceLoc())
{

}

FloatLiteralExpr::FloatLiteralExpr(FloatType val, const SourceLoc& begLoc, const SourceLoc& endLoc) 
	: val_(val), Expr(ExprKind::FloatLiteralExpr,begLoc,endLoc)
{

}

FloatType FloatLiteralExpr::getVal() const
{
	return val_;
}

void FloatLiteralExpr::setVal(FloatType val)
{
	val_ = val;
}

// Literals : String literals
StringLiteralExpr::StringLiteralExpr() : StringLiteralExpr("",SourceLoc(),SourceLoc())
{

}

StringLiteralExpr::StringLiteralExpr(const std::string & val, const SourceLoc& begLoc, const SourceLoc& endLoc) 
	: val_(val), Expr(ExprKind::StringLiteralExpr,begLoc,endLoc)
{

}

std::string StringLiteralExpr::getVal() const
{
	return val_;
}

void StringLiteralExpr::setVal(const std::string & val)
{
	val_ = val;
}

// Literals : Bool literals
BoolLiteralExpr::BoolLiteralExpr() : BoolLiteralExpr(false,SourceLoc(),SourceLoc())
{

}

BoolLiteralExpr::BoolLiteralExpr(bool val, const SourceLoc& begLoc, const SourceLoc& endLoc)
	: val_(val), Expr(ExprKind::BoolLiteralExpr,begLoc,endLoc)
{

}

bool BoolLiteralExpr::getVal() const
{
	return val_;
}

void BoolLiteralExpr::setVal(bool val)
{
	val_ = val;
}

// Literals: Array literals
ArrayLiteralExpr::ArrayLiteralExpr() : ArrayLiteralExpr(ExprVector(), SourceLoc(), SourceLoc())
{
}

ArrayLiteralExpr::ArrayLiteralExpr(ExprVector&& exprs, const SourceLoc& begLoc, const SourceLoc& endLoc)
	: exprs_(exprs), Expr(ExprKind::ArrayLiteralExpr,begLoc,endLoc)
{

}

ExprVector& ArrayLiteralExpr::getExprs()
{
	return exprs_;
}

const ExprVector& ArrayLiteralExpr::getExprs() const
{
	return exprs_;
}

void ArrayLiteralExpr::setExprs(ExprVector&& exprs)
{
	exprs_ = exprs;
}

std::size_t ArrayLiteralExpr::getSize() const
{
	return exprs_.size();
}

bool ArrayLiteralExpr::isEmpty() const
{
	return (exprs_.size() == 0);
}

// BinaryExpr
BinaryExpr::BinaryExpr() : BinaryExpr(OpKind::None, nullptr,
	nullptr, SourceLoc(), SourceRange(), SourceLoc())
{

}

BinaryExpr::BinaryExpr(OpKind op, Expr* lhs, Expr* rhs, const SourceLoc& begLoc, const SourceRange& opRange, const SourceLoc& endLoc) :
	op_(op), Expr(ExprKind::BinaryExpr, begLoc, endLoc), opRange_(opRange), lhs_(lhs), rhs_(rhs)
{

}

Expr* BinaryExpr::getLHS() const
{
	return lhs_;
}

Expr* BinaryExpr::getRHS() const
{
	return rhs_;
}

void BinaryExpr::setLHS(Expr* expr)
{
	lhs_ = expr;
}

void BinaryExpr::setRHS(Expr* expr)
{
	rhs_ = expr;
}

BinaryExpr::OpKind BinaryExpr::getOp() const
{
	return op_;
}

void BinaryExpr::setOp(OpKind op)
{
	op_ = op;
}

SourceRange BinaryExpr::getOpRange() const
{
	return opRange_;
}

std::string BinaryExpr::getOpSign(OpKind op)
{
	switch (op)
	{
		case OpKind::None:
			return "";
		#define BINARY_OP(ID, SIGN, NAME) case OpKind::ID: return SIGN;
		#include "Operators.def"
		default:
			fox_unreachable("Unknown binary operator kind");
	}
}

std::string BinaryExpr::getOpID(OpKind op)
{
	switch (op)
	{
		case OpKind::None:
			return "None";
		#define BINARY_OP(ID, SIGN, NAME) case OpKind::ID: return #ID;
		#include "Operators.def"
		default:
			fox_unreachable("Unknown binary operator kind");
	}
}

std::string BinaryExpr::getOpName(OpKind op)
{
	switch (op)
	{
		case OpKind::None:
			return "None";
		#define BINARY_OP(ID, SIGN, NAME) case OpKind::ID: return NAME;
		#include "Operators.def"
		default:
			fox_unreachable("Unknown binary operator kind");
	}
}

// UnaryExpr
UnaryExpr::UnaryExpr() : UnaryExpr(OpKind::None, nullptr,
	SourceLoc(), SourceRange(), SourceLoc())
{

}

UnaryExpr::UnaryExpr(OpKind op, Expr* expr, const SourceLoc& begLoc, const SourceRange& opRange, const SourceLoc& endLoc)
	: op_(op), Expr(ExprKind::UnaryExpr,begLoc,endLoc), opRange_(opRange), expr_(expr)
{
}

Expr* UnaryExpr::getExpr() const
{
	return expr_;
}

void UnaryExpr::setExpr(Expr* expr)
{
	expr_ = expr;
}

UnaryExpr::OpKind UnaryExpr::getOp() const
{
	return op_;
}

void UnaryExpr::setOp(OpKind op)
{
	op_ = op;
}

SourceRange UnaryExpr::getOpRange() const
{
	return opRange_;
}

std::string UnaryExpr::getOpSign(OpKind op)
{
	switch (op)
	{
		case OpKind::None:
			return "";
		#define UNARY_OP(ID, SIGN, NAME) case OpKind::ID: return SIGN;
		#include "Operators.def"
		default:
			fox_unreachable("Unknown unary operator kind");
	}
}

std::string UnaryExpr::getOpID(OpKind op)
{
	switch (op)
	{
		case OpKind::None:
			return "None";
		#define UNARY_OP(ID, SIGN, NAME) case OpKind::ID: return #ID;
		#include "Operators.def"
		default:
			fox_unreachable("Unknown unary operator kind");
	}
}

std::string UnaryExpr::getOpName(OpKind op)
{
	switch (op)
	{
		case OpKind::None:
			return "None";
		#define UNARY_OP(ID, SIGN, NAME) case OpKind::ID: return NAME;
		#include "Operators.def"
		default:
			fox_unreachable("Unknown unary operator kind");
	}
}

// CastExpr
CastExpr::CastExpr() : CastExpr(nullptr,nullptr,SourceLoc(),SourceRange(),SourceLoc())
{

}

CastExpr::CastExpr(Type* castGoal, Expr* expr,const SourceLoc& begLoc, const SourceRange& typeRange, const SourceLoc& endLoc):
	goal_(castGoal), expr_(expr), Expr(ExprKind::CastExpr,begLoc,endLoc), typeRange_(typeRange)
{

}

void CastExpr::setCastGoal(Type* goal)
{
	goal_ = goal;
}

Type* CastExpr::getCastGoal() const
{
	return goal_;
}

Expr* CastExpr::getExpr() const
{
	return expr_;
}

void CastExpr::setExpr(Expr* expr)
{
	expr_ = expr;
}

SourceRange CastExpr::getTypeRange() const
{
	return typeRange_;
}

// DeclRefs
DeclRefExpr::DeclRefExpr() : DeclRefExpr(nullptr,SourceLoc(),SourceLoc())
{
}

DeclRefExpr::DeclRefExpr(IdentifierInfo * declid, const SourceLoc& begLoc, const SourceLoc& endLoc)
	: declId_(declid), Expr(ExprKind::DeclRefExpr,begLoc,endLoc)
{

}

IdentifierInfo * DeclRefExpr::getIdentifier() const
{
	return declId_;
}

void DeclRefExpr::setDeclIdentifier(IdentifierInfo * id)
{
	declId_ = id;
}

// function call
FunctionCallExpr::FunctionCallExpr() : FunctionCallExpr(nullptr, ExprVector(), SourceLoc(), SourceLoc())
{

}

FunctionCallExpr::FunctionCallExpr(Expr* callee, ExprVector&& args, const SourceLoc& begLoc, const SourceLoc& endLoc):
	callee_(callee), args_(args), Expr(ExprKind::FunctionCallExpr,begLoc,endLoc)
{
}

Expr* FunctionCallExpr::getCallee() const
{
	return callee_;
}

void FunctionCallExpr::setCallee(Expr* callee)
{
	callee_ = callee;
}

ExprVector& FunctionCallExpr::getArgs()
{
	return args_;
}

const ExprVector& FunctionCallExpr::getArgs() const
{
	return args_;
}

void FunctionCallExpr::setArgs(ExprVector&& args)
{
	args_ = args;
}

ExprVector::iterator FunctionCallExpr::args_begin()
{
	return args_.begin();
}

ExprVector::const_iterator FunctionCallExpr::args_begin() const
{
	return args_.begin();
}

ExprVector::iterator FunctionCallExpr::args_end()
{
	return args_.end();
}

ExprVector::const_iterator FunctionCallExpr::args_end() const
{
	return args_.end();
}

// MemberOf Expr
MemberOfExpr::MemberOfExpr() : MemberOfExpr(nullptr,nullptr,SourceLoc(),SourceLoc(),SourceLoc())
{
}

MemberOfExpr::MemberOfExpr(Expr* base, IdentifierInfo * idInfo,
	const SourceLoc& begLoc, const SourceLoc& dotLoc, const SourceLoc& endLoc) 
	: Expr(ExprKind::MemberOfExpr,begLoc,endLoc), base_(base), membName_(idInfo), dotLoc_(dotLoc)
{

}

Expr* MemberOfExpr::getBase() const
{
	return base_;
}

void MemberOfExpr::setBase(Expr* expr)
{
	base_ = expr;
}

IdentifierInfo* MemberOfExpr::getMemberID() const
{
	return membName_;
}

void MemberOfExpr::setMemberName(IdentifierInfo* idInfo)
{
	membName_ = idInfo;
}

SourceLoc MemberOfExpr::getDotLoc() const
{
	return dotLoc_;
}

// Array Access
ArrayAccessExpr::ArrayAccessExpr() : ArrayAccessExpr(nullptr,nullptr,SourceLoc(),SourceLoc())
{

}

ArrayAccessExpr::ArrayAccessExpr(Expr* expr, Expr* idxexpr, const SourceLoc& begLoc, const SourceLoc& endLoc) :
	base_(expr), idxExpr_(idxexpr), Expr(ExprKind::ArrayAccessExpr,begLoc,endLoc)
{
	
}

void ArrayAccessExpr::setBase(Expr* expr)
{
	base_ = expr;
}

void ArrayAccessExpr::setAccessIndexExpr(Expr* expr)
{
	idxExpr_ = expr;
}

Expr* ArrayAccessExpr::getBase() const
{
	return base_;
}

Expr* ArrayAccessExpr::getAccessIndexExpr() const
{
	return idxExpr_;
}

// Parens Expr
ParensExpr::ParensExpr() : ParensExpr(nullptr, SourceLoc(), SourceLoc())
{

}

ParensExpr::ParensExpr(Expr* expr, const SourceLoc & begLoc, const SourceLoc & endLoc) 
	: Expr(ExprKind::ParensExpr,begLoc,endLoc), expr_(expr)
{

}

Expr* ParensExpr::getExpr() const
{
	return expr_;
}

void ParensExpr::setExpr(Expr* expr)
{
	expr_ = expr;
}