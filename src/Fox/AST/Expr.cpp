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

#include <tuple>
#include <map>
#include <sstream> 
#include <cassert>

using namespace fox;

// operators
static const std::map<BinaryOperator, std::pair<std::string,std::string>> kBinopToStr_dict =
{
	{ BinaryOperator::DEFAULT			, {" " , "!INVALID!"}},
	{ BinaryOperator::LOGIC_AND			, {"&&", "LOGICAL AND"} },
	{ BinaryOperator::CONCAT			, {"+" , "CONCAT"}},
	{ BinaryOperator::LOGIC_OR			, {"||", "LOGICAL OR"}},
	{ BinaryOperator::ADD				, {"+" , "ADDITION"}},
	{ BinaryOperator::MINUS				, {"-" , "SUBSTRACTION"}},
	{ BinaryOperator::MUL				, {"*" , "MULTIPLICATION"}},
	{ BinaryOperator::DIV				, {"/" , "DIVISION"}},
	{ BinaryOperator::MOD				, {"%" , "MODULO"}},
	{ BinaryOperator::EXP				, {"**", "EXPONENT" }},
	{ BinaryOperator::LESS_OR_EQUAL		, {"<=", "LESS OR EQUAL THAN"}},
	{ BinaryOperator::GREATER_OR_EQUAL	, {">=", "GREATER OR EQUAL THAN"}},
	{ BinaryOperator::LESS_THAN			, {"<", "LESS THAN"}},
	{ BinaryOperator::GREATER_THAN		, {">", "GREATER THAN"}},
	{ BinaryOperator::EQUAL				, {"==", "EQUAL"}},
	{ BinaryOperator::NOTEQUAL			, {"!=", "NOT EQUAL"}},
	{ BinaryOperator::ASSIGN_BASIC		, {"=", "ASSIGN"}}
};

static const std::map<UnaryOperator, std::pair<std::string,std::string>> kUnaryOpToStr_dict =
{
	{ UnaryOperator::DEFAULT	, {" ", "!INVALID!"}},
	{ UnaryOperator::LOGICNOT	, {"!", "LOGICAL NOT"}},
	{ UnaryOperator::NEGATIVE	, {"-", "NEGATIVE"}},
	{ UnaryOperator::POSITIVE	, {"+", "POSITIVE"}}
};

std::string operators::toString(const BinaryOperator & op)
{
	auto it = kBinopToStr_dict.find(op);
	assert((it != kBinopToStr_dict.end()) && "Unknown operator?");
	return it->second.first;
}

std::string operators::toString(const UnaryOperator & op)
{
	auto it = kUnaryOpToStr_dict.find(op);
	assert((it != kUnaryOpToStr_dict.end()) && "Unknown operator?");
	return it->second.first;
}

std::string operators::getName(const BinaryOperator & op)
{
	auto it = kBinopToStr_dict.find(op);
	assert((it != kBinopToStr_dict.end()) && "Unknown operator?");
	return it->second.second;
}

std::string operators::getName(const UnaryOperator & op)
{
	auto it = kUnaryOpToStr_dict.find(op);
	assert((it != kUnaryOpToStr_dict.end()) && "Unknown operator?");
	return it->second.second;
}

// Expr
Expr::Expr(StmtKind ekind, const SourceLoc& begLoc, const SourceLoc& endLoc) : Stmt(ekind,begLoc,endLoc)
{

}

// Literals : Char literals
CharLiteralExpr::CharLiteralExpr() : CharLiteralExpr(0, SourceLoc(), SourceLoc())
{

}

CharLiteralExpr::CharLiteralExpr(CharType val, const SourceLoc& begLoc, const SourceLoc& endLoc) 
	: val_(val), Expr(StmtKind::CharLiteralExpr,begLoc,endLoc)
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
	: val_(val), Expr(StmtKind::IntegerLiteralExpr,begLoc,endLoc)
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
	: val_(val), Expr(StmtKind::FloatLiteralExpr,begLoc,endLoc)
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
	: val_(val), Expr(StmtKind::StringLiteralExpr,begLoc,endLoc)
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
	: val_(val), Expr(StmtKind::BoolLiteralExpr,begLoc,endLoc)
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
	: exprs_(exprs), Expr(StmtKind::ArrayLiteralExpr,begLoc,endLoc)
{

}

ExprVector& ArrayLiteralExpr::getArgs()
{
	return exprs_;
}

const ExprVector& ArrayLiteralExpr::getArgs() const
{
	return exprs_;
}

void ArrayLiteralExpr::setArgs(ExprVector&& exprs)
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
BinaryExpr::BinaryExpr() : BinaryExpr(BinaryOperator::DEFAULT,nullptr,nullptr,SourceLoc(),SourceRange(),SourceLoc())
{

}

BinaryExpr::BinaryExpr(BinaryOperator opt, Expr* lhs, Expr* rhs, const SourceLoc& begLoc, const SourceRange& opRange, const SourceLoc& endLoc) :
	op_(opt), Expr(StmtKind::BinaryExpr, begLoc, endLoc), opRange_(opRange), lhs_(lhs), rhs_(rhs)
{

}

Expr* BinaryExpr::getLHS()
{
	return lhs_;
}

Expr* BinaryExpr::getRHS()
{
	return rhs_;
}

const Expr* BinaryExpr::getLHS() const
{
	return lhs_;
}

const Expr* BinaryExpr::getRHS() const
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

BinaryOperator BinaryExpr::getOp() const
{
	return op_;
}

void BinaryExpr::setOp(BinaryOperator op)
{
	op_ = op;
}

SourceRange BinaryExpr::getOpRange() const
{
	return opRange_;
}

// UnaryExpr
UnaryExpr::UnaryExpr() : UnaryExpr(UnaryOperator::DEFAULT,nullptr,SourceLoc(),SourceRange(),SourceLoc())
{

}

UnaryExpr::UnaryExpr(UnaryOperator opt, Expr* expr, const SourceLoc& begLoc, const SourceRange& opRange, const SourceLoc& endLoc)
	: op_(opt), Expr(StmtKind::UnaryExpr,begLoc,endLoc), opRange_(opRange), expr_(expr)
{
}

Expr* UnaryExpr::getExpr()
{
	return expr_;
}

const Expr* UnaryExpr::getExpr() const
{
	return expr_;
}

void UnaryExpr::setExpr(Expr* expr)
{
	expr_ = expr;
}

UnaryOperator UnaryExpr::getOp() const
{
	return op_;
}

void UnaryExpr::setOp(UnaryOperator nop)
{
	op_ = nop;
}

SourceRange UnaryExpr::getOpRange() const
{
	return opRange_;
}

// CastExpr
CastExpr::CastExpr() : CastExpr(nullptr,nullptr,SourceLoc(),SourceRange(),SourceLoc())
{

}

CastExpr::CastExpr(Type* castGoal, Expr* expr,const SourceLoc& begLoc, const SourceRange& typeRange, const SourceLoc& endLoc):
	goal_(castGoal), expr_(expr), Expr(StmtKind::CastExpr,begLoc,endLoc), typeRange_(typeRange)
{

}

void CastExpr::setCastGoal(Type* goal)
{
	goal_ = goal;
}

Type* CastExpr::getCastGoal()
{
	return goal_;
}

const Type* CastExpr::getCastGoal() const
{
	return goal_;
}

Expr* CastExpr::getExpr()
{
	return expr_;
}

const Expr* CastExpr::getExpr() const
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
	: declId_(declid), Expr(StmtKind::DeclRefExpr,begLoc,endLoc)
{

}

IdentifierInfo * DeclRefExpr::getIdentifier()
{
	return declId_;
}

const IdentifierInfo * DeclRefExpr::getIdentifier() const
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
	callee_(callee), args_(args), Expr(StmtKind::FunctionCallExpr,begLoc,endLoc)
{
}

Expr* FunctionCallExpr::getCallee()
{
	return callee_;
}

const Expr* FunctionCallExpr::getCallee() const
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

// MemberOf Expr
MemberOfExpr::MemberOfExpr() : MemberOfExpr(nullptr,nullptr,SourceLoc(),SourceLoc(),SourceLoc())
{
}

MemberOfExpr::MemberOfExpr(Expr* base, IdentifierInfo * idInfo,
	const SourceLoc& begLoc, const SourceLoc& dotLoc, const SourceLoc& endLoc) 
	: Expr(StmtKind::MemberOfExpr,begLoc,endLoc), base_(base), membName_(idInfo), dotLoc_(dotLoc)
{

}

Expr* MemberOfExpr::getBase()
{
	return base_;
}

const Expr* MemberOfExpr::getBase() const
{
	return base_;
}

void MemberOfExpr::setBase(Expr* expr)
{
	base_ = expr;
}

IdentifierInfo * MemberOfExpr::getMemberID()
{
	return membName_;
}

const IdentifierInfo* MemberOfExpr::getMemberID() const
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
	base_(expr), idxExpr_(idxexpr), Expr(StmtKind::ArrayAccessExpr,begLoc,endLoc)
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

Expr* ArrayAccessExpr::getBase()
{
	return base_;
}

Expr* ArrayAccessExpr::getAccessIndexExpr()
{
	return idxExpr_;
}

const Expr* ArrayAccessExpr::getBase() const
{
	return base_;
}

const Expr* ArrayAccessExpr::getAccessIndexExpr() const
{
	return idxExpr_;
}

// Parens Expr
ParensExpr::ParensExpr() : ParensExpr(nullptr, SourceLoc(), SourceLoc())
{

}

ParensExpr::ParensExpr(Expr* expr, const SourceLoc & begLoc, const SourceLoc & endLoc) 
	: Expr(StmtKind::ParensExpr,begLoc,endLoc), expr_(expr)
{

}

Expr* ParensExpr::getExpr()
{
	return expr_;
}

const Expr* ParensExpr::getExpr() const
{
	return expr_;
}

void ParensExpr::setExpr(Expr* expr)
{
	expr_ = expr;
}