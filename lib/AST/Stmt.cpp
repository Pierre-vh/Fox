//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
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

Stmt::Stmt(StmtKind skind, SourceRange range):
  kind_(skind), range_(range) {}

StmtKind Stmt::getKind() const {
  return kind_;
}

SourceRange Stmt::getRange() const {
  return range_;
}

SourceLoc Stmt::getBegin() const {
  return range_.getBegin();
}

SourceLoc Stmt::getEnd() const {
  return range_.getEnd();
}

void* Stmt::operator new(std::size_t sz, ASTContext& ctxt, std::uint8_t align) {
  return ctxt.allocate(sz, align);
}

void* Stmt::operator new(std::size_t, void* mem) {
  assert(mem);
  return mem;
}

void Stmt::setRange(SourceRange range) {
  range_ = range;
}

//----------------------------------------------------------------------------//
// NullStmt
//----------------------------------------------------------------------------//

NullStmt::NullStmt(SourceLoc semiLoc):
  Stmt(StmtKind::NullStmt, SourceRange(semiLoc)) {

}

NullStmt* NullStmt::create(ASTContext& ctxt, SourceLoc semiLoc) {
  return new(ctxt) NullStmt(semiLoc);
}

void NullStmt::setSemiLoc(SourceLoc semiLoc) {
  setRange(SourceRange(semiLoc));
}

SourceLoc NullStmt::getSemiLoc() const {
  return getRange().getBegin();
}

//----------------------------------------------------------------------------//
// ReturnStmt
//----------------------------------------------------------------------------//

ReturnStmt::ReturnStmt(Expr* rtr_expr, SourceRange range):
  Stmt(StmtKind::ReturnStmt, range), expr_(rtr_expr) {}

bool ReturnStmt::hasExpr() const {
  return (bool)expr_;
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

ConditionStmt::ConditionStmt(Expr* cond, ASTNode then, ASTNode elsenode,
  SourceRange range, SourceLoc ifHeaderEndLoc): Stmt(StmtKind::ConditionStmt,
  range), cond_(cond), then_(then),  else_(elsenode), 
  ifHeadEndLoc_(ifHeaderEndLoc) {}

ConditionStmt* 
ConditionStmt::create(ASTContext& ctxt, Expr* cond, ASTNode then, ASTNode condElse,
  SourceRange range, SourceLoc ifHeaderEnd) {
  return new(ctxt) ConditionStmt(cond, then, condElse, range, ifHeaderEnd);
}

bool ConditionStmt::isValid() const {
  return cond_ && then_;
}

bool ConditionStmt::hasElse() const {
  return (bool)else_;
}

Expr* ConditionStmt::getCond() const {
  return cond_;
}

ASTNode ConditionStmt::getThen() const {
  return then_;
}

ASTNode ConditionStmt::getElse() const {
  return else_;
}

void ConditionStmt::setCond(Expr* expr) {
  cond_ = expr;
}

void ConditionStmt::setThen(ASTNode node) {
  then_ = node;
}

void ConditionStmt::setElse(ASTNode node) {
  else_ = node;
}

void ConditionStmt::setIfHeaderEndLoc(SourceLoc sloc) {
  ifHeadEndLoc_ = sloc;
}

SourceRange ConditionStmt::getIfHeaderRange() const {
  return SourceRange(getBegin(), ifHeadEndLoc_);
}

SourceLoc ConditionStmt::getIfHeaderEndLoc() const {
  return ifHeadEndLoc_;
}

//----------------------------------------------------------------------------//
// CompoundStmt
//----------------------------------------------------------------------------//

CompoundStmt::CompoundStmt(ArrayRef<ASTNode> elems, SourceRange range):
  Stmt(StmtKind::CompoundStmt, range), 
  numNodes_(static_cast<SizeTy>(elems.size())) {
  assert((numNodes_ < maxNodes) && "Too many elements for CompoundStmt. "
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

std::size_t CompoundStmt::getSize() const {
  return numNodes_;
}

//----------------------------------------------------------------------------//
// WhileStmt
//----------------------------------------------------------------------------//

WhileStmt::WhileStmt(Expr* cond, ASTNode body, SourceRange range, 
  SourceLoc headerEndLoc): Stmt(StmtKind::WhileStmt, range),
 headerEndLoc_(headerEndLoc), cond_(cond), body_(body) {}

Expr* WhileStmt::getCond() const {
  return cond_;
}

ASTNode WhileStmt::getBody() const {
  return body_;
}

WhileStmt* WhileStmt::create(ASTContext& ctxt, Expr* cond, ASTNode body,
  SourceRange range, SourceLoc headerEnd) {
  return new(ctxt) WhileStmt(cond, body, range, headerEnd);
}

void WhileStmt::setCond(Expr* cond) {
  cond_ = cond;
}

void WhileStmt::setBody(ASTNode body) {
  body_ = body;
}

SourceLoc WhileStmt::getHeaderEndLoc() const {
  return headerEndLoc_;
}

SourceRange WhileStmt::getHeaderRange() const {
  return SourceRange(getBegin(), headerEndLoc_);
}