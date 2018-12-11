//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : Decl.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/AST/Expr.hpp"
#include "Fox/AST/Decl.hpp"
#include "Fox/AST/Stmt.hpp"
#include "Fox/Common/Source.hpp"
#include "Fox/AST/ASTContext.hpp"
#include <sstream>
#include <cassert>

using namespace fox;

//----------------------------------------------------------------------------//
// Decl
//----------------------------------------------------------------------------//

Decl::Decl(DeclKind kind, DeclContext* parent, SourceRange range):
  kind_(kind), range_(range), parent_(parent) {}

DeclKind Decl::getKind() const {
  return kind_;
}

DeclContext* Decl::getDeclContext() const {
  return parent_;
}

DeclContext* Decl::getClosestDeclContext() const {
  if(auto* dc = dyn_cast<DeclContext>(const_cast<Decl*>(this)))
    return dc;
  return getDeclContext();
}

void Decl::setRange(SourceRange range) {
  range_ = range;
}

SourceRange Decl::getRange() const {
  return range_;
}

FileID Decl::getFile() const {
  return range_.getBegin().getFileID();
}

void* Decl::operator new(std::size_t sz, ASTContext& ctxt, std::uint8_t align) {
  return ctxt.getAllocator().allocate(sz, align);
}

//----------------------------------------------------------------------------//
// NamedDecl
//----------------------------------------------------------------------------//

NamedDecl::NamedDecl(DeclKind kind, DeclContext* parent, Identifier id, 
  SourceRange range): Decl(kind, parent, range), identifier_(id) {}

Identifier NamedDecl::getIdentifier() const {
  return identifier_;
}

void NamedDecl::setIdentifier(Identifier id) {
  identifier_ = id;
}

bool NamedDecl::hasIdentifier() const {
  return !identifier_.isNull();
}

//----------------------------------------------------------------------------//
// ValueDecl
//----------------------------------------------------------------------------//

ValueDecl::ValueDecl(DeclKind kind, DeclContext* parent, Identifier id,
  TypeLoc ty, bool isConst, SourceRange range): 
  NamedDecl(kind, parent, id, range), isConst_(isConst), type_(ty) {}

Type ValueDecl::getType() const {
  return type_.withoutLoc();
}

TypeLoc ValueDecl::getTypeLoc() const {
  return type_;
}

void ValueDecl::setTypeLoc(TypeLoc ty) {
  type_ = ty;
}

SourceRange ValueDecl::getTypeRange() const {
  return type_.getRange();
}

bool ValueDecl::isConstant() const {
  return isConst_;
}

void ValueDecl::setIsConstant(bool k) {
  isConst_ = k;
}

//----------------------------------------------------------------------------//
// ParamDecl
//----------------------------------------------------------------------------//

ParamDecl::ParamDecl():
  ParamDecl(nullptr, Identifier(), TypeLoc(), false, SourceRange()) {

}

ParamDecl::ParamDecl(DeclContext* parent, Identifier id, TypeLoc type,
  bool isConst, SourceRange range):
  ValueDecl(DeclKind::ParamDecl, parent, id, type, isConst, range) {

}

//----------------------------------------------------------------------------//
// FuncDecl
//----------------------------------------------------------------------------//

FuncDecl::FuncDecl(DeclContext* parent, TypeLoc returnType, Identifier fnId,
  CompoundStmt* body, SourceRange range, SourceLoc headerEndLoc):
  NamedDecl(DeclKind::FuncDecl, parent, fnId, range), 
  headEndLoc_(headerEndLoc), body_(body), returnType_(returnType), 
  DeclContext(DeclContextKind::FuncDecl) {}

void FuncDecl::setLocs(SourceRange range, SourceLoc headerEndLoc) {
  setRange(range);
  setHeaderEndLoc(headerEndLoc);
}

void FuncDecl::setHeaderEndLoc(SourceLoc loc) {
  headEndLoc_ = loc;
}

SourceLoc FuncDecl::getHeaderEndLoc() const {
  return headEndLoc_;
}

SourceRange FuncDecl::getHeaderRange() const {
  return SourceRange(getRange().getBegin(), headEndLoc_);
}

void FuncDecl::setReturnTypeLoc(TypeLoc ty) {
  returnType_ = ty;
}

TypeLoc FuncDecl::getReturnTypeLoc() const {
  return returnType_;
}

Type FuncDecl::getReturnType() const {
  return returnType_.withoutLoc();
}

SourceRange FuncDecl::getReturnTypeRange() const {
  return returnType_.getRange();
}

CompoundStmt* FuncDecl::getBody() const {
  return body_;
}

void FuncDecl::setBody(CompoundStmt* body) {
  body_ = body;
}

ParamDecl* FuncDecl::getParam(std::size_t ind) const {
  assert(ind < params_.size() && "out-of-range");
  return params_[ind];
}

FuncDecl::ParamVecTy& FuncDecl::getParams() {
  return params_;
}

void FuncDecl::addParam(ParamDecl* param) {
  params_.push_back(param);
}

void FuncDecl::setParam(ParamDecl* param, std::size_t idx) {
  assert(idx <= params_.size() && "Out of range");
  params_[idx] = param;
}

void FuncDecl::setParams(ParamVecTy&& params) {
  params_ = params;
}

std::size_t FuncDecl::getNumParams() const {
  return params_.size();
}

//----------------------------------------------------------------------------//
// VarDecl
//----------------------------------------------------------------------------//

VarDecl::VarDecl(DeclContext* parent, Identifier id, TypeLoc type, 
  bool isConst, Expr* init, SourceRange range):
  ValueDecl(DeclKind::VarDecl, parent, id, type, isConst, range),
  init_(init) {}

Expr* VarDecl::getInitExpr() const {
  return init_;
}

bool VarDecl::hasInitExpr() const {
  return (bool)init_;
}

void VarDecl::setInitExpr(Expr* expr) {
  init_ = expr;
}

//----------------------------------------------------------------------------//
// UnitDecl
//----------------------------------------------------------------------------//

UnitDecl::UnitDecl(ASTContext& ctxt, DeclContext* parent, Identifier id,
  FileID inFile): NamedDecl(DeclKind::UnitDecl, parent, id, SourceRange()),
  file_(inFile), DeclContext(DeclContextKind::UnitDecl), ctxt_(ctxt) {}

FileID UnitDecl::getFileID() const {
  return file_;
}

void UnitDecl::setFileID(const FileID& fid) {
  file_ = fid;
}

ASTContext& fox::UnitDecl::getASTContext() {
  return ctxt_;
}
