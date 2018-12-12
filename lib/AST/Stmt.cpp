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

Stmt::Stmt(StmtKind skind, SourceRange range):
  kind_(skind), range_(range) {}

StmtKind Stmt::getKind() const {
  return kind_;
}

SourceRange Stmt::getRange() const {
  return range_;
}

void* Stmt::operator new(std::size_t sz, ASTContext& ctxt, std::uint8_t align) {
  return ctxt.getAllocator().allocate(sz, align);
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
  return SourceRange(getRange().getBegin(), ifHeadEndLoc_);
}

SourceLoc ConditionStmt::getIfHeaderEndLoc() const {
  return ifHeadEndLoc_;
}

//----------------------------------------------------------------------------//
// CompoundStmt
//----------------------------------------------------------------------------//

CompoundStmt::CompoundStmt(SourceRange range):
  Stmt(StmtKind::CompoundStmt, range) {}

ASTNode CompoundStmt::getNode(std::size_t ind) const {
  assert(ind < nodes_.size() && "out-of-range");
  return nodes_[ind];
}

CompoundStmt::NodeVecTy& CompoundStmt::getNodes() {
  return nodes_;
}

CompoundStmt* CompoundStmt::create(ASTContext& ctxt, SourceRange range) {
  return new(ctxt) CompoundStmt(range);
}

void CompoundStmt::addNode(ASTNode node) {
  nodes_.push_back(node);
}

void CompoundStmt::setNode(ASTNode node, std::size_t idx) {
  assert((idx < nodes_.size()) && "Out of range");
  nodes_[idx] = node;
}

bool CompoundStmt::isEmpty() const {
  return !(nodes_.size());
}

std::size_t CompoundStmt::size() const {
  return nodes_.size();
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
  return SourceRange(getRange().getBegin(), headerEndLoc_);
}