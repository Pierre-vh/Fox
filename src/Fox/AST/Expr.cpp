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
ArrayLiteralExpr::ArrayLiteralExpr() : ArrayLiteralExpr(nullptr,SourceLoc(),SourceLoc())
{
}

ArrayLiteralExpr::ArrayLiteralExpr(std::unique_ptr<ExprList> exprs, const SourceLoc& begLoc, const SourceLoc& endLoc)
	: exprs_(std::move(exprs)), Expr(StmtKind::ArrayLiteralExpr,begLoc,endLoc)
{

}

ExprList * ArrayLiteralExpr::getExprList()
{
	return exprs_.get();
}

void ArrayLiteralExpr::setExprList(std::unique_ptr<ExprList> elist)
{
	exprs_ = std::move(elist);
}

bool ArrayLiteralExpr::hasExprList() const
{
	return (bool)exprs_;
}

bool ArrayLiteralExpr::isEmpty() const
{
	if (exprs_)
		return (exprs_->size() == 0); // -> has exprs but size == 0 -> empty
	return false; // No exprs -> it's empty
}

// BinaryExpr
BinaryExpr::BinaryExpr() : BinaryExpr(BinaryOperator::DEFAULT,nullptr,nullptr,SourceLoc(),SourceRange(),SourceLoc())
{

}

BinaryExpr::BinaryExpr(BinaryOperator opt, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs, const SourceLoc& begLoc, const SourceRange& opRange, const SourceLoc& endLoc) :
	op_(opt), Expr(StmtKind::BinaryExpr, begLoc, endLoc), opRange_(opRange)
{
	setLHS(std::move(lhs));
	setRHS(std::move(rhs));
}

Expr* BinaryExpr::getLHS()
{
	return left_.get();
}

Expr* BinaryExpr::getRHS()
{
	return right_.get();
}

const Expr* BinaryExpr::getLHS() const
{
	return left_.get();
}

const Expr* BinaryExpr::getRHS() const
{
	return right_.get();
}

void BinaryExpr::setLHS(std::unique_ptr<Expr> nlhs)
{
	left_ = std::move(nlhs);
}

void BinaryExpr::setRHS(std::unique_ptr<Expr> nrhs)
{
	right_ = std::move(nrhs);
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

UnaryExpr::UnaryExpr(UnaryOperator opt, std::unique_ptr<Expr> node, const SourceLoc& begLoc, const SourceRange& opRange, const SourceLoc& endLoc)
	: op_(opt), Expr(StmtKind::UnaryExpr,begLoc,endLoc), opRange_(opRange), child_(std::move(node))
{
}

Expr* UnaryExpr::getChild()
{
	return child_.get();
}

const Expr* UnaryExpr::getChild() const
{
	return child_.get();
}

void UnaryExpr::setChild(std::unique_ptr<Expr> nchild)
{
	child_ = std::move(nchild);
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

CastExpr::CastExpr(Type* castGoal, std::unique_ptr<Expr> child,const SourceLoc& begLoc, const SourceRange& typeRange, const SourceLoc& endLoc):
	goal_(castGoal), child_(std::move(child)), Expr(StmtKind::CastExpr,begLoc,endLoc), typeRange_(typeRange)
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

Expr* CastExpr::getChild()
{
	return child_.get();
}

const Expr* CastExpr::getChild() const
{
	return child_.get();
}

void CastExpr::setChild(std::unique_ptr<Expr> nc)
{
	child_ = std::move(nc);
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
FunctionCallExpr::FunctionCallExpr() : FunctionCallExpr(nullptr, nullptr, SourceLoc(), SourceLoc())
{

}

FunctionCallExpr::FunctionCallExpr(std::unique_ptr<Expr> base, std::unique_ptr<ExprList> elist, const SourceLoc& begLoc, const SourceLoc& endLoc):
	callee_(std::move(base)), args_(std::move(elist)), Expr(StmtKind::FunctionCallExpr,begLoc,endLoc)
{
}

Expr* FunctionCallExpr::getCallee()
{
	return callee_.get();
}

const Expr* FunctionCallExpr::getCallee() const
{
	return callee_.get();
}

void FunctionCallExpr::setCallee(std::unique_ptr<Expr> base)
{
	callee_ = std::move(base);
}

ExprList* FunctionCallExpr::getExprList()
{
	return args_.get();
}

const ExprList* FunctionCallExpr::getExprList() const
{
	return args_.get();
}

void FunctionCallExpr::setExprList(std::unique_ptr<ExprList> elist)
{
	args_ = std::move(elist);
}

// MemberOf Expr
MemberOfExpr::MemberOfExpr() : MemberOfExpr(nullptr,nullptr,SourceLoc(),SourceLoc(),SourceLoc())
{
}

MemberOfExpr::MemberOfExpr(std::unique_ptr<Expr> base, IdentifierInfo * idInfo, const SourceLoc& begLoc, const SourceLoc& dotLoc, const SourceLoc& endLoc) 
	: Expr(StmtKind::MemberOfExpr,begLoc,endLoc), base_(std::move(base)), membName_(idInfo), dotLoc_(dotLoc)
{

}

Expr* MemberOfExpr::getBase()
{
	return base_.get();
}

const Expr * MemberOfExpr::getBase() const
{
	return base_.get();
}

void MemberOfExpr::setBase(std::unique_ptr<Expr> expr)
{
	base_ = std::move(expr);
}

IdentifierInfo * MemberOfExpr::getMemberID()
{
	return membName_;
}

const IdentifierInfo * MemberOfExpr::getMemberID() const
{
	return membName_;
}

void MemberOfExpr::setMemberName(IdentifierInfo * idInfo)
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

ArrayAccessExpr::ArrayAccessExpr(std::unique_ptr<Expr> expr, std::unique_ptr<Expr> idxexpr, const SourceLoc& begLoc, const SourceLoc& endLoc) :
	base_(std::move(expr)), accessIdxExpr_(std::move(idxexpr)), Expr(StmtKind::ArrayAccessExpr,begLoc,endLoc)
{
	
}

void ArrayAccessExpr::setBase(std::unique_ptr<Expr> expr)
{
	base_ = std::move(expr);
}

void ArrayAccessExpr::setAccessIndexExpr(std::unique_ptr<Expr> expr)
{
	accessIdxExpr_ = std::move(expr);
}

Expr* ArrayAccessExpr::getBase()
{
	return base_.get();
}

Expr* ArrayAccessExpr::getAccessIndexExpr()
{
	return accessIdxExpr_.get();
}

const Expr* ArrayAccessExpr::getBase() const
{
	return base_.get();
}

const Expr* ArrayAccessExpr::getAccessIndexExpr() const
{
	return accessIdxExpr_.get();
}

// Expr list
void ExprList::addExpr(std::unique_ptr<Expr> expr)
{
	exprs_.emplace_back(std::move(expr));
}

Expr* ExprList::getExpr(std::size_t ind)
{
	assert(ind < exprs_.size() && "out-of-range");
	return exprs_[ind].get();
}

const Expr* ExprList::getExpr(std::size_t ind) const
{
	assert(ind < exprs_.size() && "out-of-range");
	return exprs_[ind].get();
}


bool ExprList::isEmpty() const
{
	return !exprs_.size();
}

std::size_t ExprList::size() const
{
	return exprs_.size();
}

ExprList::ExprListIter ExprList::begin()
{
	return exprs_.begin();
}

ExprList::ExprListIter ExprList::end()
{
	return exprs_.end();
}

ExprList::ExprListConstIter ExprList::begin() const
{
	return exprs_.begin();
}

ExprList::ExprListConstIter ExprList::end() const
{
	return exprs_.end();
}

// Parens Expr
ParensExpr::ParensExpr() : ParensExpr(nullptr, SourceLoc(), SourceLoc())
{

}

ParensExpr::ParensExpr(std::unique_ptr<Expr> expr, const SourceLoc & begLoc, const SourceLoc & endLoc) 
	: Expr(StmtKind::ParensExpr,begLoc,endLoc), expr_(std::move(expr))
{

}

Expr* ParensExpr::getExpr()
{
	return expr_.get();
}

const Expr* ParensExpr::getExpr() const
{
	return expr_.get();
}

void ParensExpr::setExpr(std::unique_ptr<Expr> expr)
{
	expr_ = std::move(expr);
}