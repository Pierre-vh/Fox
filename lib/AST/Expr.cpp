//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : Expr.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/AST/Expr.hpp"
#include "Fox/AST/ASTContext.hpp"
#include "Fox/AST/Types.hpp"
#include "Fox/Common/Errors.hpp"
#include <sstream> 
#include <utility>

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

#define EXPR(ID, PARENT)\
  static_assert(std::is_trivially_destructible<ID>::value, \
  #ID " is allocated in the ASTContext: It's destructor is never called!");
#include "Fox/AST/ExprNodes.def"

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
  return ctxt.allocate(sz, align);
}

void* Expr::operator new(std::size_t, void* mem) {
  assert(mem); 
  return mem;
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

//----------------------------------------------------------------------------//
// StringLiteralExpr 
//----------------------------------------------------------------------------//

StringLiteralExpr::StringLiteralExpr(string_view val, SourceRange range):
  val_(val), Expr(ExprKind::StringLiteralExpr, range) {

}

StringLiteralExpr* 
StringLiteralExpr::create(ASTContext& ctxt, string_view val,
  SourceRange range) {
  return new(ctxt) StringLiteralExpr(val, range);
}

string_view StringLiteralExpr::getVal() const {
  return val_;
}

//----------------------------------------------------------------------------//
// BoolLiteralExpr 
//----------------------------------------------------------------------------//

BoolLiteralExpr::BoolLiteralExpr(bool val, SourceRange range):
  val_(val), Expr(ExprKind::BoolLiteralExpr, range) {

}

BoolLiteralExpr* 
BoolLiteralExpr::create(ASTContext& ctxt, bool val, SourceRange range) {
  return new(ctxt) BoolLiteralExpr(val, range);
}

bool BoolLiteralExpr::getVal() const {
  return val_;
}

//----------------------------------------------------------------------------//
// ArrayLiteralExpr 
//----------------------------------------------------------------------------//

ArrayLiteralExpr::ArrayLiteralExpr(ArrayRef<Expr*> elems, SourceRange range):
  Expr(ExprKind::ArrayLiteralExpr, range), 
  numElems_(static_cast<SizeTy>(elems.size())) {
  assert((elems.size() < maxElems) && "Too many args for ArrayLiteralExpr. "
    "Change the type of SizeTy to something bigger!");

  std::uninitialized_copy(elems.begin(), elems.end(), 
    getTrailingObjects<Expr*>());
}

ArrayLiteralExpr* 
ArrayLiteralExpr::create(ASTContext& ctxt, ArrayRef<Expr*> elems,
  SourceRange range) {
  auto totalSize = totalSizeToAlloc<Expr*>(elems.size());
  void* mem = ctxt.allocate(totalSize, alignof(ArrayLiteralExpr));
  return new(mem) ArrayLiteralExpr(elems, range);
}

MutableArrayRef<Expr*> ArrayLiteralExpr::getExprs() {
  return {getTrailingObjects<Expr*>(), numElems_};
}

ArrayRef<Expr*> ArrayLiteralExpr::getExprs() const {
  return {getTrailingObjects<Expr*>(), numElems_};
}

Expr* ArrayLiteralExpr::getExpr(std::size_t idx) const {
  assert((idx < numElems_) && "Out of range");
  return getExprs()[idx];
}

void ArrayLiteralExpr::setExpr(Expr* expr, std::size_t idx) {
  assert((idx < numElems_) && "Out of range");
  getExprs()[idx] = expr;
}

std::size_t ArrayLiteralExpr::numElems() const {
  return numElems_;
}

bool ArrayLiteralExpr::isEmpty() const {
  return (numElems_ == 0);
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
  Expr(ExprKind::CastExpr, range), goal_(castGoal), 
  exprAndIsUseless_(expr, false) {}

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
  exprAndIsUseless_.setPointer(expr);
}

Expr* CastExpr::getExpr() const {
  return exprAndIsUseless_.getPointer();
}

bool CastExpr::isUseless() const {
  return exprAndIsUseless_.getInt();
}

void CastExpr::markAsUselesss() {
  exprAndIsUseless_.setInt(true);
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

ValueDecl* DeclRefExpr::getDecl() const {
  return decl_;
}

void DeclRefExpr::setDecl(ValueDecl* decl) {
  decl_ = decl;
}

//----------------------------------------------------------------------------//
// CallExpr 
//----------------------------------------------------------------------------//

CallExpr::CallExpr(Expr* callee, ArrayRef<Expr*> args, 
  SourceRange range): Expr(ExprKind::CallExpr, range), callee_(callee), 
  numArgs_(static_cast<SizeTy>(args.size())) {
  assert((args.size() < maxArgs) && "Too many args for CallExpr. "
    "Change the type of SizeTy to something bigger!");
  std::uninitialized_copy(args.begin(), args.end(), 
    getTrailingObjects<Expr*>());
}

CallExpr* 
CallExpr::create(ASTContext& ctxt, Expr* callee, ArrayRef<Expr*> args, 
  SourceRange range) {
  auto totalSize = totalSizeToAlloc<Expr*>(args.size());
  void* mem = ctxt.allocate(totalSize, alignof(CallExpr));
  return new(mem) CallExpr(callee, args, range);
}

void CallExpr::setCallee(Expr* callee) {
  callee_ = callee;
}

Expr* CallExpr::getCallee() const {
  return callee_;
}

CallExpr::SizeTy CallExpr::numArgs() const {
  return numArgs_;
}

MutableArrayRef<Expr*> CallExpr::getArgs() {
  return { getTrailingObjects<Expr*>(), numArgs_};
}

ArrayRef<Expr*> CallExpr::getArgs() const {
  return { getTrailingObjects<Expr*>(), numArgs_};
}

Expr* CallExpr::getArg(std::size_t idx) const {
  assert((idx < numArgs_) && "Out of range");
  return getArgs()[idx];
}

void CallExpr::setArg(Expr* arg, std::size_t idx) {
  assert((idx < numArgs_) && "Out of range");
  getArgs()[idx] = arg;
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

ArraySubscriptExpr::ArraySubscriptExpr(Expr* expr, Expr* idxexpr, 
  SourceRange range): base_(expr), idxExpr_(idxexpr), 
  Expr(ExprKind::ArraySubscriptExpr, range) {}

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
