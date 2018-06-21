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

using namespace Moonshot;

// Operators
static const std::map<binaryOperator, std::pair<std::string,std::string>> kBinopToStr_dict =
{
	{ binaryOperator::DEFAULT			, {" " , "!INVALID!"}},
	{ binaryOperator::LOGIC_AND			, {"&&", "LOGICAL AND"} },
	{ binaryOperator::CONCAT			, {"+" , "CONCAT"}},
	{ binaryOperator::LOGIC_OR			, {"||", "LOGICAL OR"}},
	{ binaryOperator::ADD				, {"+" , "ADDITION"}},
	{ binaryOperator::MINUS				, {"-" , "SUBSTRACTION"}},
	{ binaryOperator::MUL				, {"*" , "MULTIPLICATION"}},
	{ binaryOperator::DIV				, {"/" , "DIVISION"}},
	{ binaryOperator::MOD				, {"%" , "MODULO"}},
	{ binaryOperator::EXP				, {"**", "EXPONENT" }},
	{ binaryOperator::LESS_OR_EQUAL		, {"<=", "LESS OR EQUAL THAN"}},
	{ binaryOperator::GREATER_OR_EQUAL	, {">=", "GREATER OR EQUAL THAN"}},
	{ binaryOperator::LESS_THAN			, {"<", "LESS THAN"}},
	{ binaryOperator::GREATER_THAN		, {">", "GREATER THAN"}},
	{ binaryOperator::EQUAL				, {"==", "EQUAL"}},
	{ binaryOperator::NOTEQUAL			, {"!=", "NOT EQUAL"}},
	{ binaryOperator::ASSIGN_BASIC		, {"=", "ASSIGN"}}
};

static const std::map<unaryOperator, std::pair<std::string,std::string>> kUnaryOpToStr_dict =
{
	{ unaryOperator::DEFAULT	, {" ", "!INVALID!"}},
	{ unaryOperator::LOGICNOT	, {"!", "LOGICAL NOT"}},
	{ unaryOperator::NEGATIVE	, {"-", "NEGATIVE"}},
	{ unaryOperator::POSITIVE	, {"+", "POSITIVE"}}
};

std::string Operators::toString(const binaryOperator & op)
{
	auto it = kBinopToStr_dict.find(op);
	assert((it != kBinopToStr_dict.end()) && "Unknown operator?");
	return it->second.first;
}

std::string Operators::toString(const unaryOperator & op)
{
	auto it = kUnaryOpToStr_dict.find(op);
	assert((it != kUnaryOpToStr_dict.end()) && "Unknown operator?");
	return it->second.first;
}

std::string Operators::getName(const binaryOperator & op)
{
	auto it = kBinopToStr_dict.find(op);
	assert((it != kBinopToStr_dict.end()) && "Unknown operator?");
	return it->second.second;
}

std::string Operators::getName(const unaryOperator & op)
{
	auto it = kUnaryOpToStr_dict.find(op);
	assert((it != kUnaryOpToStr_dict.end()) && "Unknown operator?");
	return it->second.second;
}

// Expr
Expr::Expr(const StmtKind & ekind, const SourceLoc& begLoc, const SourceLoc& endLoc) : Stmt(ekind,begLoc,endLoc)
{

}

bool Expr::isValid() const
{
	return hasLocInfo();
}

// NullExpr
NullExpr::NullExpr() : NullExpr(SourceLoc())
{

}

NullExpr::NullExpr(const SourceLoc& semiLoc) : Expr(StmtKind::NullExpr, semiLoc, semiLoc), isRecoveryNode_(semiLoc.isValid())
{

}

bool NullExpr::isRecoveryNullExpr() const
{
	return isRecoveryNode_;
}

// Literals : Char literals
CharLiteralExpr::CharLiteralExpr() : CharLiteralExpr(0, SourceLoc(), SourceLoc())
{

}

CharLiteralExpr::CharLiteralExpr(const CharType & val, const SourceLoc& begLoc, const SourceLoc& endLoc) 
	: val_(val), Expr(StmtKind::CharLiteralExpr,begLoc,endLoc)
{

}

CharType CharLiteralExpr::getVal() const
{
	return val_;
}

void CharLiteralExpr::setVal(const CharType & val)
{
	val_ = val;
}

// Literals : Integer literals
IntegerLiteralExpr::IntegerLiteralExpr() : IntegerLiteralExpr(0,SourceLoc(),SourceLoc())
{

}

IntegerLiteralExpr::IntegerLiteralExpr(const IntType & val, const SourceLoc& begLoc, const SourceLoc& endLoc)
	: val_(val), Expr(StmtKind::IntegerLiteralExpr,begLoc,endLoc)
{

}

IntType IntegerLiteralExpr::getVal() const
{
	return val_;
}

void IntegerLiteralExpr::setVal(const IntType & val)
{
	val_ = val;
}

// Literals : Float literals
FloatLiteralExpr::FloatLiteralExpr(const FloatType & val, const SourceLoc& begLoc, const SourceLoc& endLoc) 
	: val_(val), Expr(StmtKind::FloatLiteralExpr,begLoc,endLoc)
{

}

FloatType FloatLiteralExpr::getVal() const
{
	return val_;
}

void FloatLiteralExpr::setVal(const FloatType & val)
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

BoolLiteralExpr::BoolLiteralExpr(const bool & val, const SourceLoc& begLoc, const SourceLoc& endLoc)
	: val_(val), Expr(StmtKind::BoolLiteralExpr,begLoc,endLoc)
{

}

bool BoolLiteralExpr::getVal() const
{
	return val_;
}

void BoolLiteralExpr::setVal(const bool & val)
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

bool ArrayLiteralExpr::isValid() const
{
	return exprs_ && Expr::isValid();
}

// BinaryExpr
BinaryExpr::BinaryExpr() : BinaryExpr(binaryOperator::DEFAULT,nullptr,nullptr,SourceLoc(),SourceRange(),SourceLoc())
{

}

BinaryExpr::BinaryExpr(const binaryOperator & opt, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs, const SourceLoc& begLoc, const SourceRange& opRange, const SourceLoc& endLoc) :
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

binaryOperator BinaryExpr::getOp() const
{
	return op_;
}

void BinaryExpr::setOp(const binaryOperator & op)
{
	op_ = op;
}

SourceRange BinaryExpr::getOpRange() const
{
	return opRange_;
}

bool BinaryExpr::isValid() const
{
	return left_ && right_ && (op_ != binaryOperator::DEFAULT) && Expr::isValid();
}

// UnaryExpr
UnaryExpr::UnaryExpr() : UnaryExpr(unaryOperator::DEFAULT,nullptr,SourceLoc(),SourceRange(),SourceLoc())
{

}

UnaryExpr::UnaryExpr(const unaryOperator & opt, std::unique_ptr<Expr> node, const SourceLoc& begLoc, const SourceRange& opRange, const SourceLoc& endLoc)
	: op_(opt), Expr(StmtKind::UnaryExpr,begLoc,endLoc), opRange_(opRange), child_(std::move(node))
{
}

bool UnaryExpr::isValid() const
{
	return opRange_.isValid() && child_ && (op_ != unaryOperator::DEFAULT) && Expr::isValid();
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

unaryOperator UnaryExpr::getOp() const
{
	return op_;
}

void UnaryExpr::setOp(const unaryOperator & nop)
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

bool CastExpr::isValid() const
{
	return typeRange_ && goal_ && child_ && Expr::isValid();
}

void CastExpr::setCastGoal(Type* goal)
{
	assert(goal && "Goal type cannot be null!");
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
DeclRefExpr::DeclRefExpr(IdentifierInfo * declid, const SourceLoc& begLoc, const SourceLoc& endLoc)
	: declId_(declid), Expr(StmtKind::DeclRefExpr,begLoc,endLoc)
{

}

IdentifierInfo * DeclRefExpr::getIdentifier()
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

bool FunctionCallExpr::isValid() const
{
	return callee_ && args_ && Expr::isValid();
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

bool MemberOfExpr::isValid() const
{
	return dotLoc_ && base_ && membName_ && Expr::isValid();
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

bool ArrayAccessExpr::isValid() const
{
	return base_ && accessIdxExpr_ && Expr::isValid();
}

// Expr list
void ExprList::addExpr(std::unique_ptr<Expr> expr)
{
	exprs_.emplace_back(std::move(expr));
}

Expr* ExprList::getExpr(const std::size_t & ind)
{
	assert(ind < exprs_.size() && "Tried to access an out of bounds location in an expression list.");
	return exprs_[ind].get();
}

const Expr* ExprList::getExpr(const std::size_t & ind) const
{
	assert(ind < exprs_.size() && "Tried to access an out of bounds location in an expression list.");
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

bool ParensExpr::isValid() const
{
	return expr_ && Expr::isValid();
}