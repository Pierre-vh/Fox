//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
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
  #ID " is allocated in the ASTContext: Its destructor is never called!");
#include "Fox/AST/ExprNodes.def"

Expr::Expr(ExprKind kind): kind_(kind){}

ExprKind Expr::getKind() const {
  return kind_;
}

namespace {
  template<typename Rtr, typename Class>
  constexpr bool isOverridenFromExpr(Rtr (Class::*)() const) {
    return true;
  }

  template<typename Rtr>
  constexpr bool isOverridenFromExpr(Rtr (Expr::*)() const) {
    return false;
  }
}

SourceRange Expr::getSourceRange() const {
  switch(getKind()) {
    #define ASSERT_HAS_GETRANGE(ID)\
      static_assert(isOverridenFromExpr(&ID::getSourceRange),\
        #ID " does not reimplement getSourceRange()")
    #define EXPR(ID, PARENT) case ExprKind::ID:\
      ASSERT_HAS_GETRANGE(ID); \
      return cast<ID>(this)->getSourceRange();
    #include "Fox/AST/ExprNodes.def"
    #undef ASSERT_HAS_GETRANGE
    default:
      fox_unreachable("all kinds handled");
  }
}

SourceLoc Expr::getBeginLoc() const {
  return getSourceRange().getBeginLoc();
}

SourceLoc Expr::getEndLoc() const {
  return getSourceRange().getEndLoc();
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
// AnyLiteralExpr 
//----------------------------------------------------------------------------//

SourceRange AnyLiteralExpr::getSourceRange() const {
  return range_;
}

AnyLiteralExpr::AnyLiteralExpr(ExprKind kind, SourceRange range):
  Expr(kind), range_(range){}

//----------------------------------------------------------------------------//
// CharLiteralExpr 
//----------------------------------------------------------------------------//

CharLiteralExpr::CharLiteralExpr(FoxChar val, SourceRange range):
  val_(val), AnyLiteralExpr(ExprKind::CharLiteralExpr, range) {}

CharLiteralExpr* 
CharLiteralExpr::create(ASTContext& ctxt, FoxChar val, SourceRange range) {
  return new(ctxt) CharLiteralExpr(val, range);
}

FoxChar CharLiteralExpr::getValue() const {
  return val_;
}

//----------------------------------------------------------------------------//
// IntegerLiteralExpr 
//----------------------------------------------------------------------------//

IntegerLiteralExpr::IntegerLiteralExpr(FoxInt val, SourceRange range):
  val_(val), AnyLiteralExpr(ExprKind::IntegerLiteralExpr, range) {}

IntegerLiteralExpr* 
IntegerLiteralExpr::create(ASTContext& ctxt, FoxInt val, SourceRange range) {
  return new(ctxt) IntegerLiteralExpr(val, range);
}

FoxInt IntegerLiteralExpr::getValue() const {
  return val_;
}

//----------------------------------------------------------------------------//
// DoubleLiteralExpr 
//----------------------------------------------------------------------------//

DoubleLiteralExpr::DoubleLiteralExpr(FoxDouble val, SourceRange range):
  val_(val), AnyLiteralExpr(ExprKind::DoubleLiteralExpr, range) {}

DoubleLiteralExpr* 
DoubleLiteralExpr::create(ASTContext& ctxt, FoxDouble val, SourceRange range) {
  return new(ctxt) DoubleLiteralExpr(val, range);
}

FoxDouble DoubleLiteralExpr::getValue() const {
  return val_;
}

//----------------------------------------------------------------------------//
// StringLiteralExpr 
//----------------------------------------------------------------------------//

StringLiteralExpr::StringLiteralExpr(string_view val, SourceRange range):
  val_(val), AnyLiteralExpr(ExprKind::StringLiteralExpr, range) {}

StringLiteralExpr* 
StringLiteralExpr::create(ASTContext& ctxt, string_view val,
  SourceRange range) {
  return new(ctxt) StringLiteralExpr(val, range);
}

string_view StringLiteralExpr::getValue() const {
  return val_;
}

//----------------------------------------------------------------------------//
// BoolLiteralExpr 
//----------------------------------------------------------------------------//

BoolLiteralExpr::BoolLiteralExpr(bool val, SourceRange range):
  val_(val), AnyLiteralExpr(ExprKind::BoolLiteralExpr, range) {}

BoolLiteralExpr* 
BoolLiteralExpr::create(ASTContext& ctxt, bool val, SourceRange range) {
  return new(ctxt) BoolLiteralExpr(val, range);
}

bool BoolLiteralExpr::getValue() const {
  return val_;
}

//----------------------------------------------------------------------------//
// ArrayLiteralExpr 
//----------------------------------------------------------------------------//

ArrayLiteralExpr::ArrayLiteralExpr(ArrayRef<Expr*> elems, SourceRange range):
  AnyLiteralExpr(ExprKind::ArrayLiteralExpr, range), 
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

BinaryExpr::BinaryExpr(OpKind op, Expr* lhs, Expr* rhs, SourceRange opRange):
  op_(op), Expr(ExprKind::BinaryExpr), opRange_(opRange), lhs_(lhs), rhs_(rhs) {}

BinaryExpr* 
BinaryExpr::create(ASTContext& ctxt, OpKind op, Expr* lhs, Expr* rhs, 
  SourceRange opRange) {
  return new(ctxt) BinaryExpr(op, lhs, rhs, opRange);
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

bool BinaryExpr::isPower() const {
  return op_ == OpKind::Pow;
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

SourceRange BinaryExpr::getSourceRange() const {
  assert(lhs_ && rhs_ && "ill formed BinaryExpr");
  return SourceRange(lhs_->getBeginLoc(), rhs_->getEndLoc());
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

UnaryExpr::UnaryExpr(OpKind op, Expr* child, SourceRange opRange):
  Expr(ExprKind::UnaryExpr), opRange_(opRange), opAndChild_(child, op) {}

UnaryExpr* UnaryExpr::create(ASTContext& ctxt, OpKind op, Expr* child, 
                             SourceRange opRange) {
  return new(ctxt) UnaryExpr(op, child, opRange);
}

void UnaryExpr::setChild(Expr* expr) {
  opAndChild_.setPointer(expr);
}

Expr* UnaryExpr::getChild() const {
  return opAndChild_.getPointer();
}

UnaryExpr::OpKind UnaryExpr::getOp() const {
  return opAndChild_.getInt();
}

void UnaryExpr::setOp(OpKind op) {
  opAndChild_.setInt(op);
}

SourceRange UnaryExpr::getOpRange() const {
  return opRange_;
}

SourceRange UnaryExpr::getSourceRange() const {
  assert(opRange_ && getChild() && "ill formed UnaryExpr");
  return SourceRange(opRange_.getBeginLoc(), getChild()->getEndLoc());
}

std::string UnaryExpr::getOpSign() const {
  switch (getOp()) {
    #define UNARY_OP(ID, SIGN, NAME) case OpKind::ID: return SIGN;
    #include "Fox/AST/Operators.def"
    default:
      fox_unreachable("Unknown unary operator kind");
  }
}

std::string UnaryExpr::getOpID() const {
  switch (getOp()) {
    #define UNARY_OP(ID, SIGN, NAME) case OpKind::ID: return #ID;
    #include "Fox/AST/Operators.def"
    default:
      fox_unreachable("Unknown unary operator kind");
  }
}

std::string UnaryExpr::getOpName() const {
  switch (getOp()) {
    #define UNARY_OP(ID, SIGN, NAME) case OpKind::ID: return NAME;
    #include "Fox/AST/Operators.def"
    default:
      fox_unreachable("Unknown unary operator kind");
  }
}

//----------------------------------------------------------------------------//
// CastExpr 
//----------------------------------------------------------------------------//

CastExpr::CastExpr(TypeLoc castGoal, Expr* child): Expr(ExprKind::CastExpr), 
  goal_(castGoal), childAndIsUseless_(child, false) {}

CastExpr* CastExpr::create(ASTContext& ctxt, TypeLoc castGoal, Expr* child) {
  return new(ctxt) CastExpr(castGoal, child);
}

void CastExpr::setCastTypeLoc(TypeLoc goal) {
  goal_ = goal;
}

TypeLoc CastExpr::getCastTypeLoc() const {
  return goal_;
}

void CastExpr::setChild(Expr* expr) {
  childAndIsUseless_.setPointer(expr);
}

Expr* CastExpr::getChild() const {
  return childAndIsUseless_.getPointer();
}

SourceRange CastExpr::getSourceRange() const {
  SourceRange castTLRange = getCastTypeLoc().getSourceRange();
  assert(castTLRange && getChild() && "ill-formed CastExpr");
  return SourceRange(getChild()->getBeginLoc(), castTLRange.getEndLoc());
}

bool CastExpr::isUseless() const {
  return childAndIsUseless_.getInt();
}

void CastExpr::markAsUselesss() {
  childAndIsUseless_.setInt(true);
}

//----------------------------------------------------------------------------//
// DeclRefExpr 
//----------------------------------------------------------------------------//

DeclRefExpr::DeclRefExpr(ValueDecl* decl, SourceRange range): range_(range),
  decl_(decl), Expr(ExprKind::DeclRefExpr) {

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

SourceRange DeclRefExpr::getSourceRange() const {
  return range_;
}

//----------------------------------------------------------------------------//
// CallExpr 
//----------------------------------------------------------------------------//

CallExpr::CallExpr(Expr* callee, ArrayRef<Expr*> args, SourceLoc rRoBrLoc):
  Expr(ExprKind::CallExpr), rightRoBrLoc_(rRoBrLoc), callee_(callee), 
  numArgs_(static_cast<SizeTy>(args.size())) {
  assert((args.size() < maxArgs) && "Too many args for CallExpr. "
    "Change the type of SizeTy to something bigger!");
  std::uninitialized_copy(args.begin(), args.end(), 
    getTrailingObjects<Expr*>());
}

CallExpr* 
CallExpr::create(ASTContext& ctxt, Expr* callee, ArrayRef<Expr*> args, 
  SourceLoc rRoBrLoc) {
  auto totalSize = totalSizeToAlloc<Expr*>(args.size());
  void* mem = ctxt.allocate(totalSize, alignof(CallExpr));
  return new(mem) CallExpr(callee, args, rRoBrLoc);
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

SourceRange CallExpr::getArgsRange() const {
  if(!numArgs_)
    return SourceRange();
  auto args = getArgs();
  SourceLoc beg = args.front()->getBeginLoc();
  SourceLoc end = args.back()->getEndLoc();
  return SourceRange(beg, end);
}

SourceRange CallExpr::getSourceRange() const {
  assert(callee_ && rightRoBrLoc_ && "ill-formed CallExpr");
  return SourceRange(callee_->getBeginLoc(), rightRoBrLoc_);
}

//----------------------------------------------------------------------------//
// UnresolvedDotExpr 
//----------------------------------------------------------------------------//

UnresolvedDotExpr::UnresolvedDotExpr(Expr* base, Identifier membID,
  SourceRange membIDRange, SourceLoc dotLoc) : 
    UnresolvedExpr(ExprKind::UnresolvedDotExpr), 
    base_(base), memb_(membID), membRange_(membIDRange), dotLoc_(dotLoc) {}

UnresolvedDotExpr* 
UnresolvedDotExpr::create(ASTContext& ctxt, Expr* base, Identifier membID, 
  SourceRange membIDRange, SourceLoc dotLoc) {
  return new(ctxt) UnresolvedDotExpr(base, membID, membIDRange, dotLoc);
}

void UnresolvedDotExpr::setBase(Expr* expr) {
  base_ = expr;
}

Expr* UnresolvedDotExpr::getBase() const {
  return base_;
}

void UnresolvedDotExpr::setMemberID(Identifier id) {
  memb_ = id;
}

Identifier UnresolvedDotExpr::getMemberID() const {
  return memb_;
}

SourceRange UnresolvedDotExpr::getMemberIDRange() const {
  return membRange_;
}

SourceLoc UnresolvedDotExpr::getDotLoc() const {
  return dotLoc_;
}

SourceRange UnresolvedDotExpr::getSourceRange() const {
  assert(base_ && membRange_ && "ill-formed UnresolvedDotExpr");
  return SourceRange(base_->getBeginLoc(), membRange_.getEndLoc());
}

//----------------------------------------------------------------------------//
// BuiltinMemberRefExpr
//----------------------------------------------------------------------------//

BuiltinMemberRefExpr* 
BuiltinMemberRefExpr::create(ASTContext& ctxt, UnresolvedDotExpr* expr, 
                             BTMKind kind) {
  auto base = expr->getBase();
  auto membID = expr->getMemberID();
  auto membRange = expr->getMemberIDRange();
  auto dotLoc = expr->getDotLoc();
  return 
    BuiltinMemberRefExpr::create(ctxt, base, membID, membRange, dotLoc, kind);
}

BuiltinMemberRefExpr* 
BuiltinMemberRefExpr::create(ASTContext& ctxt, Expr* base, Identifier membID, 
                             SourceRange membIDRange, SourceLoc dotLoc, 
                             BTMKind kind) {
  return new(ctxt) BuiltinMemberRefExpr(base, membID, membIDRange, dotLoc, kind);
}

void BuiltinMemberRefExpr::setBase(Expr* expr) {
  base_ = expr;
}

Expr* BuiltinMemberRefExpr::getBase() const {
  return base_;
}

void BuiltinMemberRefExpr::setMemberID(Identifier id) {
  memb_ = id;
}

Identifier BuiltinMemberRefExpr::getMemberID() const {
  return memb_;
}

SourceRange BuiltinMemberRefExpr::getMemberIDRange() const {
  return membRange_;
}

SourceLoc BuiltinMemberRefExpr::getDotLoc() const {
  return dotLoc_;
}

SourceRange BuiltinMemberRefExpr::getSourceRange() const {
  assert(base_ && membRange_ && "ill-formed BuiltinMemberRefExpr");
  return SourceRange(base_->getBeginLoc(), membRange_.getEndLoc());
}

bool BuiltinMemberRefExpr::isCalled() const {
  assert(isMethod() && "only for methods");
  return isCalled_;
}

void BuiltinMemberRefExpr::setIsCalled(bool value) {
  assert(isMethod() && "only for methods");
  isCalled_ = value;
}

bool BuiltinMemberRefExpr::isMethod() const {
  return isMethod_;
}

void BuiltinMemberRefExpr::setIsMethod(bool value) {
  isMethod_ = value;
}

BuiltinTypeMemberKind BuiltinMemberRefExpr::getBuiltinTypeMemberKind() const {
  return kind_;
}

void BuiltinMemberRefExpr::setBuiltinTypeMemberKind(BuiltinTypeMemberKind value) {
  kind_ = value;
}

BuiltinMemberRefExpr::
BuiltinMemberRefExpr(Expr* base, Identifier membID, SourceRange membIDRange, 
                     SourceLoc dotLoc, BTMKind kind) :
                     Expr(ExprKind::BuiltinMemberRefExpr),
                     base_(base), kind_(kind), memb_(membID), 
                     membRange_(membIDRange), dotLoc_(dotLoc), isCalled_(false),
                     isMethod_(false) { }

//----------------------------------------------------------------------------//
// SubscriptExpr 
//----------------------------------------------------------------------------//

SubscriptExpr::SubscriptExpr(Expr* expr, Expr* idxexpr, 
SourceLoc rightBracketLoc): base_(expr), idxExpr_(idxexpr),
  rightBracketLoc_(rightBracketLoc), Expr(ExprKind::SubscriptExpr) {}

SubscriptExpr* 
SubscriptExpr::create(ASTContext& ctxt, Expr* base, Expr* idx, 
  SourceLoc rightBracketLoc) {
  return new(ctxt) SubscriptExpr(base, idx, rightBracketLoc);
}

void SubscriptExpr::setBase(Expr* expr) {
  base_ = expr;
}

Expr* SubscriptExpr::getBase() const {
  return base_;
}

void SubscriptExpr::setIndex(Expr* expr) {
  idxExpr_ = expr;
}

Expr* SubscriptExpr::getIndex() const {
  return idxExpr_;
}

SourceRange SubscriptExpr::getSourceRange() const {
  assert(base_ && rightBracketLoc_ && "ill-formed ArraySubscriptExpr");
  return SourceRange(base_->getBeginLoc(), rightBracketLoc_);
}

//----------------------------------------------------------------------------//
// UnresolvedExpr 
//----------------------------------------------------------------------------///

UnresolvedExpr::UnresolvedExpr(ExprKind kind):
Expr(kind) {}

//----------------------------------------------------------------------------//
// UnresolvedDeclRefExpr 
//----------------------------------------------------------------------------///

UnresolvedDeclRefExpr::UnresolvedDeclRefExpr(Identifier id, SourceRange range):
  UnresolvedExpr(ExprKind::UnresolvedDeclRefExpr), range_(range), id_(id) {}

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

SourceRange UnresolvedDeclRefExpr::getSourceRange() const {
  return range_;
}

//----------------------------------------------------------------------------//
// ErrorExpr
//----------------------------------------------------------------------------//

ErrorExpr* ErrorExpr::create(ASTContext& ctxt) {
  ErrorExpr* expr = new(ctxt) ErrorExpr();
  expr->setType(ErrorType::get(ctxt));
  return expr;
}

SourceRange ErrorExpr::getSourceRange() const {
  return SourceRange();
}

ErrorExpr::ErrorExpr() : Expr(ExprKind::ErrorExpr) {}
