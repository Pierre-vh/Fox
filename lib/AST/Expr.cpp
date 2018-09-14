////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Expr.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Fox/AST/Expr.hpp"
#include "Fox/AST/Identifiers.hpp"
#include "Fox/AST/ASTContext.hpp"
#include "Fox/Common/Errors.hpp"
#include <map>
#include <sstream> 

using namespace fox;

//------//
// Expr //
//------//

Expr::Expr(ExprKind kind, const SourceRange& range):
	kind_(kind), range_(range)
{

}

ExprKind Expr::getKind() const
{
	return kind_;
}

void Expr::setRange(const SourceRange& range)
{
	range_ = range;
}

SourceRange Expr::getRange() const
{
	return range_;
}

void Expr::setType(Type* type)
{
	type_ = type;
}

Type* Expr::getType()
{
	return type_;
}

const Type* Expr::getType() const
{
	return type_;
}

void* Expr::operator new(std::size_t sz, ASTContext& ctxt, std::uint8_t align)
{
	return ctxt.getAllocator().allocate(sz, align);
}


//-----------------//
// CharLiteralExpr //
//-----------------//

CharLiteralExpr::CharLiteralExpr():
	CharLiteralExpr(0, SourceRange())
{

}

CharLiteralExpr::CharLiteralExpr(FoxChar val, const SourceRange& range):
	val_(val), Expr(ExprKind::CharLiteralExpr, range)
{

}

FoxChar CharLiteralExpr::getVal() const
{
	return val_;
}

void CharLiteralExpr::setVal(FoxChar val)
{
	val_ = val;
}

//--------------------//
// IntegerLiteralExpr //
//--------------------//

IntegerLiteralExpr::IntegerLiteralExpr():
	IntegerLiteralExpr(0, SourceRange())
{

}

IntegerLiteralExpr::IntegerLiteralExpr(FoxInt val, const SourceRange& range):
	val_(val), Expr(ExprKind::IntegerLiteralExpr, range)
{

}

FoxInt IntegerLiteralExpr::getVal() const
{
	return val_;
}

void IntegerLiteralExpr::setVal(FoxInt val)
{
	val_ = val;
}

//------------------//
// FloatLiteralExpr //
//------------------//

FloatLiteralExpr::FloatLiteralExpr():
	FloatLiteralExpr(0, SourceRange())
{

}

FloatLiteralExpr::FloatLiteralExpr(FoxFloat val, const SourceRange& range):
	val_(val), Expr(ExprKind::FloatLiteralExpr, range)
{

}

FoxFloat FloatLiteralExpr::getVal() const
{
	return val_;
}

void FloatLiteralExpr::setVal(FoxFloat val)
{
	val_ = val;
}

//-------------------//
// StringLiteralExpr //
//-------------------//

StringLiteralExpr::StringLiteralExpr():
	StringLiteralExpr("", SourceRange())
{

}

StringLiteralExpr::StringLiteralExpr(const FoxString& val, const SourceRange& range):
	val_(val), Expr(ExprKind::StringLiteralExpr, range)
{

}

std::string StringLiteralExpr::getVal() const
{
	return val_;
}

void StringLiteralExpr::setVal(const FoxString& val)
{
	val_ = val;
}

//-----------------//
// BoolLiteralExpr //
//-----------------//

BoolLiteralExpr::BoolLiteralExpr():
	BoolLiteralExpr(false, SourceRange())
{

}

BoolLiteralExpr::BoolLiteralExpr(FoxBool val, const SourceRange& range):
	val_(val), Expr(ExprKind::BoolLiteralExpr, range)
{

}

FoxBool BoolLiteralExpr::getVal() const
{
	return val_;
}

void BoolLiteralExpr::setVal(FoxBool val)
{
	val_ = val;
}

//------------------//
// ArrayLiteralExpr //
//------------------//

ArrayLiteralExpr::ArrayLiteralExpr():
	ArrayLiteralExpr(ExprVector(), SourceRange())
{
}

ArrayLiteralExpr::ArrayLiteralExpr(const ExprVector& exprs, const SourceRange& range):
	exprs_(exprs), Expr(ExprKind::ArrayLiteralExpr, range)
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

Expr* ArrayLiteralExpr::getExpr(std::size_t idx)
{
	assert((idx < exprs_.size()) && "Out of range");
	return exprs_[idx];
}

const Expr* ArrayLiteralExpr::getExpr(std::size_t idx) const
{
	assert((idx < exprs_.size()) && "Out of range");
	return exprs_[idx];
}

void ArrayLiteralExpr::setExprs(ExprVector&& exprs)
{
	exprs_ = exprs;
}

void ArrayLiteralExpr::replaceExpr(Expr* expr, std::size_t idx)
{
	assert((idx < exprs_.size()) && "Out of range");
	exprs_[idx] = expr;
}

void ArrayLiteralExpr::replaceExpr(Expr* expr, ExprVector::iterator it)
{
	assert((it != exprs_.end()) && "Out of range");
	(*it) = expr;
}

std::size_t ArrayLiteralExpr::getSize() const
{
	return exprs_.size();
}

bool ArrayLiteralExpr::isEmpty() const
{
	return (exprs_.size() == 0);
}

ExprVector::iterator ArrayLiteralExpr::exprs_begin()
{
	return exprs_.begin();
}

ExprVector::const_iterator ArrayLiteralExpr::exprs_begin() const
{
	return exprs_.begin();
}

ExprVector::iterator ArrayLiteralExpr::exprs_end()
{
	return exprs_.begin();
}

ExprVector::const_iterator ArrayLiteralExpr::exprs_end() const
{
	return exprs_.begin();
}

//------------//
// BinaryExpr //
//------------//

BinaryExpr::BinaryExpr():
	BinaryExpr(OpKind::Invalid, nullptr, nullptr, SourceRange(), SourceRange())
{

}

BinaryExpr::BinaryExpr(OpKind op, Expr* lhs, Expr* rhs, 
	const SourceRange& range, const SourceRange& opRange) :
	op_(op), Expr(ExprKind::BinaryExpr, range), 
	opRange_(opRange), lhs_(lhs), rhs_(rhs)
{

}

void BinaryExpr::setLHS(Expr* expr)
{
	lhs_ = expr;
}

Expr* BinaryExpr::getLHS()
{
	return lhs_;
}

const Expr* BinaryExpr::getLHS() const
{
	return lhs_;
}

void BinaryExpr::setRHS(Expr* expr)
{
	rhs_ = expr;
}

Expr* BinaryExpr::getRHS()
{
	return rhs_;
}

const Expr* BinaryExpr::getRHS() const
{
	return rhs_;
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
		#define BINARY_OP(ID, SIGN, NAME) case OpKind::ID: return SIGN;
		#include "Fox/AST/Operators.def"
		default:
			fox_unreachable("Unknown binary operator kind");
	}
}

std::string BinaryExpr::getOpID(OpKind op)
{
	switch (op)
	{
		#define BINARY_OP(ID, SIGN, NAME) case OpKind::ID: return #ID;
		#include "Fox/AST/Operators.def"
		default:
			fox_unreachable("Unknown binary operator kind");
	}
}

std::string BinaryExpr::getOpName(OpKind op)
{
	switch (op)
	{
		#define BINARY_OP(ID, SIGN, NAME) case OpKind::ID: return NAME;
		#include "Fox/AST/Operators.def"
		default:
			fox_unreachable("Unknown binary operator kind");
	}
}

//-----------//
// UnaryExpr //
//-----------//

UnaryExpr::UnaryExpr(): UnaryExpr(OpKind::Invalid, nullptr,
	SourceRange(), SourceRange())
{

}

UnaryExpr::UnaryExpr(OpKind op, Expr* expr, 
	const SourceRange& range, const SourceRange& opRange):
	op_(op), Expr(ExprKind::UnaryExpr, range),
	opRange_(opRange), expr_(expr)
{

}

void UnaryExpr::setExpr(Expr* expr)
{
	expr_ = expr;
}

Expr* UnaryExpr::getExpr()
{
	return expr_;
}

const Expr* UnaryExpr::getExpr() const
{
	return expr_;
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
		#define UNARY_OP(ID, SIGN, NAME) case OpKind::ID: return SIGN;
		#include "Fox/AST/Operators.def"
		default:
			fox_unreachable("Unknown unary operator kind");
	}
}

std::string UnaryExpr::getOpID(OpKind op)
{
	switch (op)
	{
		#define UNARY_OP(ID, SIGN, NAME) case OpKind::ID: return #ID;
		#include "Fox/AST/Operators.def"
		default:
			fox_unreachable("Unknown unary operator kind");
	}
}

std::string UnaryExpr::getOpName(OpKind op)
{
	switch (op)
	{
		#define UNARY_OP(ID, SIGN, NAME) case OpKind::ID: return NAME;
		#include "Fox/AST/Operators.def"
		default:
			fox_unreachable("Unknown unary operator kind");
	}
}

//----------//
// CastExpr //
//----------//

CastExpr::CastExpr() : CastExpr(nullptr, nullptr, 
	SourceRange(), SourceRange())
{

}

CastExpr::CastExpr(Type* castGoal, Expr* expr, 
	const SourceRange& range, const SourceRange& typeRange):
	Expr(ExprKind::CastExpr, range),
	goal_(castGoal), expr_(expr), typeRange_(typeRange)
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

void CastExpr::setExpr(Expr* expr)
{
	expr_ = expr;
}

Expr* CastExpr::getExpr()
{
	return expr_;
}

const Expr* CastExpr::getExpr() const
{
	return expr_;
}

SourceRange CastExpr::getTypeRange() const
{
	return typeRange_;
}

//-------------//
// DeclRefExpr //
//-------------//

DeclRefExpr::DeclRefExpr() : DeclRefExpr(nullptr, SourceRange())
{
}

DeclRefExpr::DeclRefExpr(Identifier* declid, const SourceRange& range)
	: id_(declid), Expr(ExprKind::DeclRefExpr, range)
{

}

void DeclRefExpr::setIdentifier(Identifier* id)
{
	id_ = id;
}

Identifier* DeclRefExpr::getIdentifier()
{
	return id_;
}

const Identifier* DeclRefExpr::getIdentifier() const
{
	return id_;
}

//------------------//
// FunctionCallExpr //
//------------------//

FunctionCallExpr::FunctionCallExpr():
	FunctionCallExpr(nullptr, ExprVector(), SourceRange())
{

}

FunctionCallExpr::FunctionCallExpr(Expr* callee, const ExprVector& args,
	const SourceRange& range): Expr(ExprKind::FunctionCallExpr, range),
	callee_(callee), args_(args)
{

}

void FunctionCallExpr::setCallee(Expr* callee)
{
	callee_ = callee;
}

Expr* FunctionCallExpr::getCallee()
{
	return callee_;
}

const Expr* FunctionCallExpr::getCallee() const
{
	return callee_;
}

ExprVector& FunctionCallExpr::getArgs()
{
	return args_;
}

const ExprVector& FunctionCallExpr::getArgs() const
{
	return args_;
}

Expr* FunctionCallExpr::getArg(std::size_t idx)
{
	assert((idx < args_.size()) && "Out of range");
	return args_[idx];
}

const Expr* FunctionCallExpr::getArg(std::size_t idx) const
{
	assert((idx < args_.size()) && "Out of range");
	return args_[idx];
}

void FunctionCallExpr::setArgs(ExprVector&& args)
{
	args_ = args;
}

void FunctionCallExpr::replaceArg(Expr* arg, std::size_t idx)
{
	assert((idx < args_.size()) && "Out of range");
	args_[idx] = arg;
}

void FunctionCallExpr::replaceArg(Expr* arg, ExprVector::iterator it)
{
	assert((it != args_.end()) && "Out of range");
	(*it) = arg;
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

//--------------//
// MemberOfExpr //
//--------------//

MemberOfExpr::MemberOfExpr():
	MemberOfExpr(nullptr, nullptr, SourceRange(), SourceLoc())
{
}

MemberOfExpr::MemberOfExpr(Expr* base, Identifier* idInfo,
	const SourceRange& range, const SourceLoc& dotLoc):
	Expr(ExprKind::MemberOfExpr, range), base_(base), membName_(idInfo),
	dotLoc_(dotLoc)
{

}

void MemberOfExpr::setExpr(Expr* expr)
{
	base_ = expr;
}

Expr* MemberOfExpr::getExpr()
{
	return base_;
}

const Expr* MemberOfExpr::getExpr() const
{
	return base_;
}

void MemberOfExpr::setMemberID(Identifier* idInfo)
{
	membName_ = idInfo;
}

Identifier* MemberOfExpr::getMemberID()
{
	return membName_;
}

const Identifier* MemberOfExpr::getMemberID() const
{
	return membName_;
}

SourceLoc MemberOfExpr::getDotLoc() const
{
	return dotLoc_;
}

//-----------------//
// ArrayAccessExpr //
//-----------------//

ArrayAccessExpr::ArrayAccessExpr():
	ArrayAccessExpr(nullptr, nullptr, SourceRange())
{

}

ArrayAccessExpr::ArrayAccessExpr(Expr* expr, Expr* idxexpr, const SourceRange& range):
	base_(expr), idxExpr_(idxexpr), Expr(ExprKind::ArrayAccessExpr, range)
{
	
}

void ArrayAccessExpr::setExpr(Expr* expr)
{
	base_ = expr;
}

Expr* ArrayAccessExpr::getExpr()
{
	return base_;
}

const Expr* ArrayAccessExpr::getExpr() const
{
	return base_;
}

void ArrayAccessExpr::setIdxExpr(Expr* expr)
{
	idxExpr_ = expr;
}

Expr* ArrayAccessExpr::getIdxExpr()
{
	return idxExpr_;
}

const Expr* ArrayAccessExpr::getIdxExpr() const
{
	return idxExpr_;
}

//------------//
// ParensExpr //
//------------//

ParensExpr::ParensExpr():
	ParensExpr(nullptr, SourceRange())
{

}

ParensExpr::ParensExpr(Expr* expr, const SourceRange& range):
	Expr(ExprKind::ParensExpr, range), expr_(expr)
{

}

void ParensExpr::setExpr(Expr* expr)
{
	expr_ = expr;
}

Expr* ParensExpr::getExpr()
{
	return expr_;
}

const Expr* ParensExpr::getExpr() const
{
	return expr_;
}
