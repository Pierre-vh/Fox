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

SourceRange Expr::getRange() const
{
	return range_;
}

void* Expr::operator new(std::size_t sz, ASTContext& ctxt, std::uint8_t align)
{
	return ctxt.getAllocator().allocate(sz, align);
}

void Expr::setRange(const SourceRange& range)
{
	range_ = range;
}

//-----------------//
// CharLiteralExpr //
//-----------------//

CharLiteralExpr::CharLiteralExpr():
	CharLiteralExpr(0, SourceRange())
{

}

CharLiteralExpr::CharLiteralExpr(CharType val, const SourceRange& range):
	val_(val), Expr(ExprKind::CharLiteralExpr, range)
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

//--------------------//
// IntegerLiteralExpr //
//--------------------//

IntegerLiteralExpr::IntegerLiteralExpr():
	IntegerLiteralExpr(0, SourceRange())
{

}

IntegerLiteralExpr::IntegerLiteralExpr(IntType val, const SourceRange& range):
	val_(val), Expr(ExprKind::IntegerLiteralExpr, range)
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

//------------------//
// FloatLiteralExpr //
//------------------//

FloatLiteralExpr::FloatLiteralExpr():
	FloatLiteralExpr(0, SourceRange())
{

}

FloatLiteralExpr::FloatLiteralExpr(FloatType val, const SourceRange& range):
	val_(val), Expr(ExprKind::FloatLiteralExpr, range)
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

//-------------------//
// StringLiteralExpr //
//-------------------//

StringLiteralExpr::StringLiteralExpr():
	StringLiteralExpr("", SourceRange())
{

}

StringLiteralExpr::StringLiteralExpr(const std::string& val, const SourceRange& range): 
	val_(val), Expr(ExprKind::StringLiteralExpr, range)
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

//-----------------//
// BoolLiteralExpr //
//-----------------//

BoolLiteralExpr::BoolLiteralExpr():
	BoolLiteralExpr(false, SourceRange())
{

}

BoolLiteralExpr::BoolLiteralExpr(bool val, const SourceRange& range):
	val_(val), Expr(ExprKind::BoolLiteralExpr, range)
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

//------------------//
// ArrayLiteralExpr //
//------------------//

ArrayLiteralExpr::ArrayLiteralExpr():
	ArrayLiteralExpr(ExprVector(), SourceRange())
{
}

ArrayLiteralExpr::ArrayLiteralExpr(ExprVector&& exprs, const SourceRange& range):
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
		#include "Operators.def"
		default:
			fox_unreachable("Unknown binary operator kind");
	}
}

std::string BinaryExpr::getOpID(OpKind op)
{
	switch (op)
	{
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
		#define BINARY_OP(ID, SIGN, NAME) case OpKind::ID: return NAME;
		#include "Operators.def"
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
		#include "Operators.def"
		default:
			fox_unreachable("Unknown unary operator kind");
	}
}

std::string UnaryExpr::getOpID(OpKind op)
{
	switch (op)
	{
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
		#define UNARY_OP(ID, SIGN, NAME) case OpKind::ID: return NAME;
		#include "Operators.def"
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

DeclRefExpr::DeclRefExpr(IdentifierInfo* declid, const SourceRange& range)
	: id_(declid), Expr(ExprKind::DeclRefExpr, range)
{

}

void DeclRefExpr::setIdentifier(IdentifierInfo* id)
{
	id_ = id;
}

IdentifierInfo* DeclRefExpr::getIdentifier()
{
	return id_;
}

const IdentifierInfo* DeclRefExpr::getIdentifier() const
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

FunctionCallExpr::FunctionCallExpr(Expr* callee, ExprVector&& args,
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

//--------------//
// MemberOfExpr //
//--------------//

MemberOfExpr::MemberOfExpr():
	MemberOfExpr(nullptr, nullptr, SourceRange(), SourceLoc())
{
}

MemberOfExpr::MemberOfExpr(Expr* base, IdentifierInfo* idInfo,
	const SourceRange& range, const SourceLoc& dotLoc):
	Expr(ExprKind::MemberOfExpr, range), base_(base), membName_(idInfo),
	dotLoc_(dotLoc)
{

}

void MemberOfExpr::setBase(Expr* expr)
{
	base_ = expr;
}

Expr* MemberOfExpr::getBase()
{
	return base_;
}

const Expr* MemberOfExpr::getBase() const
{
	return base_;
}

void MemberOfExpr::setMemberID(IdentifierInfo* idInfo)
{
	membName_ = idInfo;
}

IdentifierInfo* MemberOfExpr::getMemberID()
{
	return membName_;
}

const IdentifierInfo* MemberOfExpr::getMemberID() const
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

void ArrayAccessExpr::setBase(Expr* expr)
{
	base_ = expr;
}

Expr* ArrayAccessExpr::getBase()
{
	return base_;
}

const Expr* ArrayAccessExpr::getBase() const
{
	return base_;
}

void ArrayAccessExpr::setAccessIndexExpr(Expr* expr)
{
	idxExpr_ = expr;
}

Expr* ArrayAccessExpr::getAccessIndexExpr()
{
	return idxExpr_;
}

const Expr* ArrayAccessExpr::getAccessIndexExpr() const
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
