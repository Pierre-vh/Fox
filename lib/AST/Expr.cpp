//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : Expr.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/AST/Expr.hpp"
#include "Fox/AST/ASTContext.hpp"
#include "Fox/Common/Errors.hpp"
#include <sstream> 

using namespace fox;

std::ostream& fox::operator<<(std::ostream& os, ExprKind kind) {
  switch (kind) {
    #define EXPR(ID, PARENT) case ExprKind::ID: os << #ID; break;
    #include "Fox/AST/ExprNodes.def"
    default:
      fox_unreachable("all kinds handled");
  }
  return os;
}

//----------------------------------------------------------------------------//
// Expr 
//----------------------------------------------------------------------------//

Expr::Expr(ExprKind kind, SourceRange range):
  kind_(kind), range_(range) {}

ExprKind Expr::getKind() const {
  return kind_;
}

void Expr::setRange(SourceRange range) {
  range_ = range;
}

SourceRange Expr::getRange() const {
  return range_;
}

SourceLoc Expr::getBegin() const {
  return range_.getBegin();
}

SourceLoc Expr::getEnd() const {
  return range_.getEnd();
}

void Expr::setType(Type type) {
  type_ = type;
}

Type Expr::getType() const {
  return type_;
}

void* Expr::operator new(std::size_t sz, ASTContext& ctxt, std::uint8_t align) {
  return ctxt.getAllocator().allocate(sz, align);
}


//----------------------------------------------------------------------------//
// CharLiteralExpr 
//----------------------------------------------------------------------------//

CharLiteralExpr::CharLiteralExpr(FoxChar val, SourceRange range):
  val_(val), Expr(ExprKind::CharLiteralExpr, range) {}

CharLiteralExpr* 
CharLiteralExpr::create(ASTContext& ctxt, FoxChar val, SourceRange range) {
  return new(ctxt) CharLiteralExpr(val, range);
}

FoxChar CharLiteralExpr::getVal() const {
  return val_;
}

void CharLiteralExpr::setVal(FoxChar val) {
  val_ = val;
}

//----------------------------------------------------------------------------//
// IntegerLiteralExpr 
//----------------------------------------------------------------------------//

IntegerLiteralExpr::IntegerLiteralExpr(FoxInt val, SourceRange range):
  val_(val), Expr(ExprKind::IntegerLiteralExpr, range) {}

IntegerLiteralExpr* 
IntegerLiteralExpr::create(ASTContext& ctxt, FoxInt val, SourceRange range) {
  return new(ctxt) IntegerLiteralExpr(val, range);
}

FoxInt IntegerLiteralExpr::getVal() const {
  return val_;
}

void IntegerLiteralExpr::setVal(FoxInt val) {
  val_ = val;
}

//----------------------------------------------------------------------------//
// FloatLiteralExpr 
//----------------------------------------------------------------------------//

FloatLiteralExpr::FloatLiteralExpr(FoxFloat val, SourceRange range):
  val_(val), Expr(ExprKind::FloatLiteralExpr, range) {}

FloatLiteralExpr* 
FloatLiteralExpr::create(ASTContext& ctxt, FoxFloat val, SourceRange range) {
  return new(ctxt) FloatLiteralExpr(val, range);
}

FoxFloat FloatLiteralExpr::getVal() const {
  return val_;
}

void FloatLiteralExpr::setVal(FoxFloat val) {
  val_ = val;
}

//----------------------------------------------------------------------------//
// StringLiteralExpr 
//----------------------------------------------------------------------------//

StringLiteralExpr::StringLiteralExpr(const FoxString& val, SourceRange range):
  val_(val), Expr(ExprKind::StringLiteralExpr, range) {

}

StringLiteralExpr* 
StringLiteralExpr::create(ASTContext& ctxt, const FoxString& val, 
  SourceRange range) {
  return new(ctxt) StringLiteralExpr(val, range);
}

std::string StringLiteralExpr::getVal() const {
  return val_;
}

void StringLiteralExpr::setVal(const FoxString& val) {
  val_ = val;
}

//----------------------------------------------------------------------------//
// BoolLiteralExpr 
//----------------------------------------------------------------------------//

BoolLiteralExpr::BoolLiteralExpr(FoxBool val, SourceRange range):
  val_(val), Expr(ExprKind::BoolLiteralExpr, range) {

}

BoolLiteralExpr* 
BoolLiteralExpr::create(ASTContext& ctxt, FoxBool val, SourceRange range) {
  return new(ctxt) BoolLiteralExpr(val, range);
}

FoxBool BoolLiteralExpr::getVal() const {
  return val_;
}

void BoolLiteralExpr::setVal(FoxBool val) {
  val_ = val;
}

//----------------------------------------------------------------------------//
// ArrayLiteralExpr 
//----------------------------------------------------------------------------//

ArrayLiteralExpr::ArrayLiteralExpr(const ExprVector& exprs, SourceRange range):
  exprs_(exprs), Expr(ExprKind::ArrayLiteralExpr, range) {

}

ArrayLiteralExpr* 
ArrayLiteralExpr::create(ASTContext& ctxt, const ExprVector& exprs, 
  SourceRange range) {
  return new(ctxt) ArrayLiteralExpr(exprs, range);
}

ExprVector& ArrayLiteralExpr::getExprs() {
  return exprs_;
}

Expr* ArrayLiteralExpr::getExpr(std::size_t idx) const {
  assert((idx < exprs_.size()) && "Out of range");
  return exprs_[idx];
}

void ArrayLiteralExpr::setExprs(ExprVector&& exprs) {
  exprs_ = exprs;
}

void ArrayLiteralExpr::setExpr(Expr* expr, std::size_t idx) {
  assert((idx < exprs_.size()) && "Out of range");
  exprs_[idx] = expr;
}

std::size_t ArrayLiteralExpr::getSize() const {
  return exprs_.size();
}

bool ArrayLiteralExpr::isEmpty() const {
  return (exprs_.size() == 0);
}

//----------------------------------------------------------------------------//
// BinaryExpr 
//----------------------------------------------------------------------------//

BinaryExpr::BinaryExpr(OpKind op, Expr* lhs, Expr* rhs, SourceRange range,
  SourceRange opRange) : op_(op), Expr(ExprKind::BinaryExpr, range), 
  opRange_(opRange), lhs_(lhs), rhs_(rhs) {}

BinaryExpr* 
BinaryExpr::create(ASTContext& ctxt, OpKind op, Expr* lhs, Expr* rhs, 
  SourceRange range, SourceRange opRange) {
  return new(ctxt) BinaryExpr(op, lhs, rhs, range, opRange);
}

void BinaryExpr::setLHS(Expr* expr) {
  lhs_ = expr;
}

Expr* BinaryExpr::getLHS() const {
  return lhs_;
}

void BinaryExpr::setRHS(Expr* expr) {
  rhs_ = expr;
}

Expr* BinaryExpr::getRHS() const {
  return rhs_;
}

BinaryExpr::OpKind BinaryExpr::getOp() const {
  return op_;
}

void BinaryExpr::setOp(OpKind op) {
  op_ = op;
}

bool BinaryExpr::isValidOp() const {
  return op_ != OpKind::Invalid;
}

bool BinaryExpr::isConcat() const {
  return op_ == OpKind::Concat;
}

bool BinaryExpr::isAdditive() const {
  switch (op_) {
    case OpKind::Add:
    case OpKind::Sub:
      return true;
    default:
      return false;
  }
}

bool BinaryExpr::isMultiplicative() const {
  switch (op_) {
    case OpKind::Mul:
    case OpKind::Div:
    case OpKind::Mod:
      return true;
    default:
      return false;
  }
}

bool BinaryExpr::isExponent() const {
  return op_ == OpKind::Exp;
}

bool BinaryExpr::isAssignement() const {
  return op_ == OpKind::Assign;
}

bool BinaryExpr::isComparison() const {
  switch (op_) {
    case OpKind::LE:
    case OpKind::GE:
    case OpKind::LT:
    case OpKind::GT:
    case OpKind::Eq:
    case OpKind::NEq:
      return true;
    default:
      return false;
  }
}

bool BinaryExpr::isRankingComparison() const {
  if (isComparison())
    return (op_ != OpKind::Eq) && (op_ != OpKind::NEq);
  return false;
}

bool BinaryExpr::isLogical() const {
  switch (op_) {
    case OpKind::LOr:
    case OpKind::LAnd:
      return true;
    default:
      return false;
  }
}

SourceRange BinaryExpr::getOpRange() const {
  return opRange_;
}

std::string BinaryExpr::getOpSign() const {
  switch (op_) {
    #define BINARY_OP(ID, SIGN, NAME) case OpKind::ID: return SIGN;
    #include "Fox/AST/Operators.def"
    default:
      fox_unreachable("Unknown binary operator kind");
  }
}

std::string BinaryExpr::getOpID() const {
  switch (op_) {
    #define BINARY_OP(ID, SIGN, NAME) case OpKind::ID: return #ID;
    #include "Fox/AST/Operators.def"
    default:
      fox_unreachable("Unknown binary operator kind");
  }
}

std::string BinaryExpr::getOpName() const {
  switch (op_) {
    #define BINARY_OP(ID, SIGN, NAME) case OpKind::ID: return NAME;
    #include "Fox/AST/Operators.def"
    default:
      fox_unreachable("Unknown binary operator kind");
  }
}

//----------------------------------------------------------------------------//
// UnaryExpr 
//----------------------------------------------------------------------------//

UnaryExpr::UnaryExpr(OpKind op, Expr* expr, SourceRange range, 
  SourceRange opRange):op_(op), Expr(ExprKind::UnaryExpr, range), 
  opRange_(opRange), expr_(expr) {}

UnaryExpr* UnaryExpr::create(ASTContext& ctxt, OpKind op, Expr* node, 
  SourceRange range, SourceRange opRange) {
  return new(ctxt) UnaryExpr(op, node, range, opRange);
}

void UnaryExpr::setExpr(Expr* expr) {
  expr_ = expr;
}

Expr* UnaryExpr::getExpr() const {
  return expr_;
}

UnaryExpr::OpKind UnaryExpr::getOp() const {
  return op_;
}

void UnaryExpr::setOp(OpKind op) {
  op_ = op;
}

SourceRange UnaryExpr::getOpRange() const {
  return opRange_;
}

std::string UnaryExpr::getOpSign() const {
  switch (op_) {
    #define UNARY_OP(ID, SIGN, NAME) case OpKind::ID: return SIGN;
    #include "Fox/AST/Operators.def"
    default:
      fox_unreachable("Unknown unary operator kind");
  }
}

std::string UnaryExpr::getOpID() const {
  switch (op_) {
    #define UNARY_OP(ID, SIGN, NAME) case OpKind::ID: return #ID;
    #include "Fox/AST/Operators.def"
    default:
      fox_unreachable("Unknown unary operator kind");
  }
}

std::string UnaryExpr::getOpName() const {
  switch (op_) {
    #define UNARY_OP(ID, SIGN, NAME) case OpKind::ID: return NAME;
    #include "Fox/AST/Operators.def"
    default:
      fox_unreachable("Unknown unary operator kind");
  }
}

//----------------------------------------------------------------------------//
// CastExpr 
//----------------------------------------------------------------------------//

CastExpr::CastExpr(TypeLoc castGoal, Expr* expr, SourceRange range):
  Expr(ExprKind::CastExpr, range), goal_(castGoal), expr_(expr) {}

CastExpr* CastExpr::create(ASTContext& ctxt, TypeLoc castGoal, 
  Expr* expr, SourceRange range) {
  return new(ctxt) CastExpr(castGoal, expr, range);
}

void CastExpr::setCastTypeLoc(TypeLoc goal) {
  goal_ = goal;
}

TypeLoc CastExpr::getCastTypeLoc() const {
  return goal_;
}

Type CastExpr::getCastType() const {
  return goal_.withoutLoc();
}

SourceRange CastExpr::getCastRange() const {
  return goal_.getRange();
}

void CastExpr::setExpr(Expr* expr) {
  expr_ = expr;
}

Expr* CastExpr::getExpr() const {
  return expr_;
}

//----------------------------------------------------------------------------//
// DeclRefExpr 
//----------------------------------------------------------------------------//

DeclRefExpr::DeclRefExpr(ValueDecl* decl, SourceRange range):
  decl_(decl), Expr(ExprKind::DeclRefExpr, range) {

}

DeclRefExpr* DeclRefExpr::create(ASTContext& ctxt, ValueDecl* decl, 
  SourceRange range) {
  return new(ctxt) DeclRefExpr(decl, range);
}

Identifier DeclRefExpr::getIdentifier() const {
  return decl_ ? decl_->getIdentifier() : Identifier();
}

ValueDecl* DeclRefExpr::getDecl() const {
  return decl_;
}

void DeclRefExpr::setDecl(ValueDecl* decl) {
  decl_ = decl;
}

//----------------------------------------------------------------------------//
// FunctionCallExpr 
//----------------------------------------------------------------------------//

FunctionCallExpr::FunctionCallExpr(Expr* callee, const ExprVector& args, 
  SourceRange range): Expr(ExprKind::FunctionCallExpr, range), callee_(callee),
  args_(args) {}

FunctionCallExpr* 
FunctionCallExpr::create(ASTContext& ctxt, Expr* callee, 
  const ExprVector& args, SourceRange range) {
  return new(ctxt) FunctionCallExpr(callee, args, range);
}

void FunctionCallExpr::setCallee(Expr* callee) {
  callee_ = callee;
}

Expr* FunctionCallExpr::getCallee() const {
  return callee_;
}

ExprVector& FunctionCallExpr::getArgs() {
  return args_;
}

Expr* FunctionCallExpr::getArg(std::size_t idx) const {
  assert((idx < args_.size()) && "Out of range");
  return args_[idx];
}

void FunctionCallExpr::setArgs(ExprVector&& args) {
  args_ = args;
}

void FunctionCallExpr::setArg(Expr* arg, std::size_t idx) {
  assert((idx < args_.size()) && "Out of range");
  args_[idx] = arg;
}

//----------------------------------------------------------------------------//
// MemberOfExpr 
//----------------------------------------------------------------------------//

MemberOfExpr::MemberOfExpr(Expr* base, Identifier membID,
  SourceRange range, SourceLoc dotLoc):
  Expr(ExprKind::MemberOfExpr, range), base_(base), memb_(membID),
  dotLoc_(dotLoc) {}

MemberOfExpr* 
MemberOfExpr::create(ASTContext& ctxt, Expr* base, Identifier membID, 
  SourceRange range, SourceLoc dotLoc) {
  return new(ctxt) MemberOfExpr(base, membID, range, dotLoc);
}

void MemberOfExpr::setExpr(Expr* expr) {
  base_ = expr;
}

Expr* MemberOfExpr::getExpr() const {
  return base_;
}

void MemberOfExpr::setMemberID(Identifier id) {
  memb_ = id;
}

Identifier MemberOfExpr::getMemberID() const {
  return memb_;
}

SourceLoc MemberOfExpr::getDotLoc() const {
  return dotLoc_;
}

//----------------------------------------------------------------------------//
// ArraySubscriptExpr 
//----------------------------------------------------------------------------//

ArraySubscriptExpr::ArraySubscriptExpr(Expr* expr, Expr* idxexpr, SourceRange range):
  base_(expr), idxExpr_(idxexpr), Expr(ExprKind::ArraySubscriptExpr, range) {}

ArraySubscriptExpr* 
ArraySubscriptExpr::create(ASTContext& ctxt, Expr* base, Expr* idx, 
  SourceRange range) {
  return new(ctxt) ArraySubscriptExpr(base, idx, range);
}

void ArraySubscriptExpr::setBase(Expr* expr) {
  base_ = expr;
}

Expr* ArraySubscriptExpr::getBase() const {
  return base_;
}

void ArraySubscriptExpr::setIndex(Expr* expr) {
  idxExpr_ = expr;
}

Expr* ArraySubscriptExpr::getIndex() const {
  return idxExpr_;
}

//----------------------------------------------------------------------------//
// UnresolvedExpr 
//----------------------------------------------------------------------------///

UnresolvedExpr::UnresolvedExpr(ExprKind kind, SourceRange range):
Expr(kind, range) {}

//----------------------------------------------------------------------------//
// UnresolvedDeclRefExpr 
//----------------------------------------------------------------------------///

UnresolvedDeclRefExpr::UnresolvedDeclRefExpr(Identifier id, SourceRange range):
  UnresolvedExpr(ExprKind::UnresolvedDeclRefExpr, range), id_(id) {}

UnresolvedDeclRefExpr* 
UnresolvedDeclRefExpr::create(ASTContext& ctxt, Identifier id, 
  SourceRange range) {
  return new(ctxt) UnresolvedDeclRefExpr(id, range);
}

void UnresolvedDeclRefExpr::setIdentifier(Identifier id) {
  id_ = id;
}

Identifier UnresolvedDeclRefExpr::getIdentifier() const {
  return id_;
}
