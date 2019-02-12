//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : Stmt.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/AST/Stmt.hpp"
#include "Fox/AST/Decl.hpp"
#include "Fox/AST/Expr.hpp"
#include "Fox/AST/ASTContext.hpp"

using namespace fox;

//----------------------------------------------------------------------------//
// Stmt
//----------------------------------------------------------------------------//

#define STMT(ID, PARENT)\
  static_assert(std::is_trivially_destructible<ID>::value, \
  #ID " is allocated in the ASTContext: It's destructor is never called!");
#include "Fox/AST/StmtNodes.def"

Stmt::Stmt(StmtKind skind): kind_(skind) {}

StmtKind Stmt::getKind() const {
  return kind_;
}

namespace {
  template<typename Rtr, typename Class>
  constexpr bool isOverridenFromStmt(Rtr (Class::*)() const) {
    return true;
  }

  template<typename Rtr>
  constexpr bool isOverridenFromStmt(Rtr (Stmt::*)() const) {
    return false;
  }
}
SourceRange Stmt::getRange() const {
  switch(getKind()) {
    #define ASSERT_HAS_GETRANGE(ID)\
      static_assert(isOverridenFromStmt(&ID::getRange),\
        #ID " does not reimplement getRange()")
    #define STMT(ID, PARENT) case StmtKind::ID:\
      ASSERT_HAS_GETRANGE(ID); \
      return cast<ID>(this)->getRange();
    #include "Fox/AST/StmtNodes.def"
    #undef ASSERT_HAS_GETRANGE
    default:
      fox_unreachable("all kinds handled");
  }
}

SourceLoc Stmt::getBegin() const {
  return getRange().getBegin();
}

SourceLoc Stmt::getEnd() const {
  return getRange().getEnd();
}

void* Stmt::operator new(std::size_t sz, ASTContext& ctxt, std::uint8_t align) {
  return ctxt.allocate(sz, align);
}

void* Stmt::operator new(std::size_t, void* mem) {
  assert(mem);
  return mem;
}

//----------------------------------------------------------------------------//
// ReturnStmt
//----------------------------------------------------------------------------//

ReturnStmt::ReturnStmt(Expr* rtr_expr, SourceRange range):
  Stmt(StmtKind::ReturnStmt), expr_(rtr_expr), range_(range) {}

bool ReturnStmt::hasExpr() const {
  return (bool)expr_;
}

SourceRange ReturnStmt::getRange() const {
  return range_;
}

Expr* ReturnStmt::getExpr() const {
  return expr_;
}

ReturnStmt* 
ReturnStmt::create(ASTContext& ctxt, Expr* rtr, SourceRange range) {
  return new(ctxt) ReturnStmt(rtr, range);
}

void ReturnStmt::setExpr(Expr* e) {
  expr_ = e;
}

//----------------------------------------------------------------------------//
// ConditionStmt
//----------------------------------------------------------------------------//

ConditionStmt::ConditionStmt(SourceLoc ifBegLoc, Expr* cond, CompoundStmt* then, 
                             CompoundStmt* elseBody): 
  Stmt(StmtKind::ConditionStmt), ifBegLoc_(ifBegLoc), cond_(cond), then_(then),
  else_(elseBody) {}

ConditionStmt* 
ConditionStmt::create(ASTContext& ctxt, SourceLoc ifBegLoc, Expr* cond, 
  CompoundStmt* then, CompoundStmt* elseBody) {
  return new(ctxt) ConditionStmt(ifBegLoc, cond, then, elseBody);
}

bool ConditionStmt::hasElse() const {
  return (bool)else_;
}

SourceRange ConditionStmt::getRange() const {
  // We should at least has a then_ node.
  assert(then_ && "ill-formed ConditionStmt");
  SourceLoc end = (else_ ? else_->getEnd() : then_->getEnd());
  return SourceRange(ifBegLoc_, end);
}

Expr* ConditionStmt::getCond() const {
  return cond_;
}

CompoundStmt* ConditionStmt::getThen() const {
  return then_;
}

CompoundStmt* ConditionStmt::getElse() const {
  return else_;
}

void ConditionStmt::setCond(Expr* expr) {
  assert(expr &&  "cannot be nullptr");
  cond_ = expr;
}

void ConditionStmt::setThen(CompoundStmt* node) {
  assert(node &&  "cannot be nullptr");
  then_ = node;
}

void ConditionStmt::setElse(CompoundStmt* node) {
  // can be nullptr
  else_ = node;
}

//----------------------------------------------------------------------------//
// CompoundStmt
//----------------------------------------------------------------------------//

CompoundStmt::CompoundStmt(ArrayRef<ASTNode> elems, SourceRange bracesRange):
  Stmt(StmtKind::CompoundStmt), bracesRange_(bracesRange), 
  numNodes_(static_cast<SizeTy>(elems.size())) {
  assert((elems.size() < maxNodes) && "Too many elements for CompoundStmt. "
    "Change the type of SizeTy to something bigger!");
  std::uninitialized_copy(elems.begin(), elems.end(), 
    getTrailingObjects<ASTNode>());
}

ASTNode CompoundStmt::getNode(std::size_t ind) const {
  assert(ind < numNodes_ && "out-of-range");
  return getNodes()[ind];
}

ArrayRef<ASTNode> CompoundStmt::getNodes() const {
  return {getTrailingObjects<ASTNode>(), numNodes_};
}

MutableArrayRef<ASTNode> CompoundStmt::getNodes() {
  return {getTrailingObjects<ASTNode>(), numNodes_};
}

CompoundStmt* CompoundStmt::create(ASTContext& ctxt, ArrayRef<ASTNode> nodes, 
  SourceRange range) {
  auto totalSize = totalSizeToAlloc<ASTNode>(nodes.size());
  void* mem = ctxt.allocate(totalSize, alignof(CompoundStmt));
  return new(mem) CompoundStmt(nodes, range);
}

void CompoundStmt::setNode(ASTNode node, std::size_t idx) {
  assert((idx < numNodes_) && "out-of-range");
  getNodes()[idx] = node;
}

bool CompoundStmt::isEmpty() const {
  return (numNodes_ == 0);
}

SourceRange CompoundStmt::getRange() const {
  return bracesRange_;
}

std::size_t CompoundStmt::getSize() const {
  return numNodes_;
}

//----------------------------------------------------------------------------//
// WhileStmt
//----------------------------------------------------------------------------//

WhileStmt::WhileStmt(SourceLoc whBegLoc, Expr* cond, CompoundStmt* body): 
  Stmt(StmtKind::WhileStmt), whBegLoc_(whBegLoc), body_(body), cond_(cond) {}

Expr* WhileStmt::getCond() const {
  return cond_;
}

CompoundStmt* WhileStmt::getBody() const {
  return body_;
}

SourceRange WhileStmt::getRange() const {
  assert(body_ && "ill formed WhileStmt");
  return SourceRange(whBegLoc_, body_->getEnd());
}

WhileStmt* WhileStmt::create(ASTContext& ctxt, SourceLoc whBegLoc, Expr* cond, 
                             CompoundStmt* body) {
  return new(ctxt) WhileStmt(whBegLoc, cond, body);
}

void WhileStmt::setCond(Expr* cond) {
  assert(cond && "cannot be nullptr!");
  cond_ = cond;
}

void WhileStmt::setBody(CompoundStmt* body) {
  assert(body && "cannot be nullptr!");
  body_ = body;
}