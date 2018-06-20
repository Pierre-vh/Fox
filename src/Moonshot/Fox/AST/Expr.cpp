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
Expr::Expr(const StmtKind & ekind) : Stmt(ekind)
{

}

// nullexpr
NullExpr::NullExpr() : Expr(StmtKind::NullExpr)
{

}

// Literals : Char literals
CharLiteralExpr::CharLiteralExpr(const CharType & val) : val_(val), Expr(StmtKind::CharLiteralExpr)
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
IntegerLiteralExpr::IntegerLiteralExpr(const IntType & val) : val_(val), Expr(StmtKind::IntegerLiteralExpr)
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
FloatLiteralExpr::FloatLiteralExpr(const FloatType & val) : val_(val), Expr(StmtKind::FloatLiteralExpr)
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
StringLiteralExpr::StringLiteralExpr(const std::string & val) : val_(val), Expr(StmtKind::StringLiteralExpr)
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
BoolLiteralExpr::BoolLiteralExpr(const bool & val) : val_(val), Expr(StmtKind::BoolLiteralExpr)
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
ArrayLiteralExpr::ArrayLiteralExpr(std::unique_ptr<ExprList> exprs) : exprs_(std::move(exprs)), Expr(StmtKind::ArrayLiteralExpr)
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
BinaryExpr::BinaryExpr(const binaryOperator & opt, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs):
	op_(opt), Expr(StmtKind::BinaryExpr)
{
	setLHS(std::move(lhs));
	setRHS(std::move(rhs));
}

std::unique_ptr<Expr> BinaryExpr::getSimple()
{
	if (left_ && !right_ && (op_ == binaryOperator::DEFAULT))	 // If the right node is empty & op == pass
	{
		auto ret = std::move(left_);
		return ret;
	}
	return nullptr;
}

Expr * BinaryExpr::getLHS()
{
	return left_.get();
}

Expr * BinaryExpr::getRHS()
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

bool BinaryExpr::isComplete() const
{
	return left_ && right_ && (op_ != binaryOperator::DEFAULT);
}

// UnaryExpr
UnaryExpr::UnaryExpr(const unaryOperator & opt, std::unique_ptr<Expr> node) : op_(opt), Expr(StmtKind::UnaryExpr)
{
	setChild(std::move(node));
}

Expr * UnaryExpr::getChild()
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

// CastExpr
CastExpr::CastExpr(Type* castGoal, std::unique_ptr<Expr> child):
	goal_(castGoal), child_(std::move(child)), Expr(StmtKind::CastExpr)
{

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

Expr * CastExpr::getChild()
{
	return child_.get();
}

void CastExpr::setChild(std::unique_ptr<Expr> nc)
{
	child_ = std::move(nc);
}

// DeclRefs
DeclRefExpr::DeclRefExpr(IdentifierInfo * declid) : declId_(declid), Expr(StmtKind::DeclRefExpr)
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
FunctionCallExpr::FunctionCallExpr(std::unique_ptr<Expr> base, std::unique_ptr<ExprList> elist):
	callee_(std::move(base)), args_(std::move(elist)), Expr(StmtKind::FunctionCallExpr)
{
}

Expr * FunctionCallExpr::getCallee()
{
	return callee_.get();
}

void FunctionCallExpr::setCallee(std::unique_ptr<Expr> base)
{
	callee_ = std::move(base);
}

ExprList * FunctionCallExpr::getExprList()
{
	return args_.get();
}

void FunctionCallExpr::setExprList(std::unique_ptr<ExprList> elist)
{
	args_ = std::move(elist);
}

// MemberOf Expr
MemberOfExpr::MemberOfExpr(std::unique_ptr<Expr> base, IdentifierInfo * idInfo) : Expr(StmtKind::MemberOfExpr), base_(std::move(base)), membName_(idInfo)
{

}

Expr * MemberOfExpr::getBase()
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

void MemberOfExpr::setMemberName(IdentifierInfo * idInfo)
{
	membName_ = idInfo;
}

// Array Access
ArrayAccessExpr::ArrayAccessExpr(std::unique_ptr<Expr> expr, std::unique_ptr<Expr> idxexpr) :
	base_(std::move(expr)), accessIdxExpr_(std::move(idxexpr)), Expr(StmtKind::ArrayAccessExpr)
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

// Expr list
void ExprList::addExpr(std::unique_ptr<Expr> expr)
{
	exprs_.emplace_back(std::move(expr));
}

Expr * ExprList::getExpr(const std::size_t & ind)
{
	if (ind > size())
		throw std::out_of_range("Tried to access an out of bounds location in an expression list.");

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
ParensExpr::ParensExpr(std::unique_ptr<Expr> expr, const SourceLoc & LParenLoc, const SourceLoc & RParenLoc) 
	: Expr(StmtKind::ParensExpr), expr_(std::move(expr)), RPLoc_(RParenLoc), LPLoc_(LParenLoc)
{

}

SourceLoc ParensExpr::getLeftParensLoc() const
{
	return LPLoc_;
}

SourceLoc ParensExpr::getRightParensLoc() const
{
	return RPLoc_;
}

void ParensExpr::setLeftParensLoc(const SourceLoc& sloc)
{
	LPLoc_ = sloc;
}

void ParensExpr::setRightParensLoc(const SourceLoc& sloc)
{
	RPLoc_ = sloc;
}

Expr* ParensExpr::getExpr()
{
	return expr_.get();
}

void ParensExpr::setExpr(std::unique_ptr<Expr> expr)
{
	expr_ = std::move(expr);
}