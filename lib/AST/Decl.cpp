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

//------//
// Decl //
//------//

Decl::Decl(DeclKind kind, SourceRange range):
  kind_(kind), range_(range) {

}

void Decl::initBitfields() {
  topLevel_ = false;
}

DeclKind Decl::getKind() const {
  return kind_;
}

bool Decl::isTopLevelDecl() const {
  return topLevel_;
}

void Decl::setIsTopLevelDecl(bool val) {
  topLevel_ = val;
}

void Decl::setRange(SourceRange range) {
  range_ = range;
}

SourceRange Decl::getRange() const {
  return range_;
}

bool fox::Decl::isValid() const {
  return range_.isValid();
}

void* Decl::operator new(std::size_t sz, ASTContext& ctxt, std::uint8_t align) {
  return ctxt.getAllocator().allocate(sz, align);
}

//-----------//
// NamedDecl //
//-----------//

NamedDecl::NamedDecl(DeclKind kind, Identifier id, SourceRange range):
  Decl(kind, range), identifier_(id) {

}

Identifier NamedDecl::getIdentifier() const {
  return identifier_;
}

void NamedDecl::setIdentifier(Identifier id) {
  identifier_ = id;
}

bool NamedDecl::hasIdentifier() const {
  return !identifier_.isNull();
}

bool NamedDecl::isValid() const {
  return Decl::isValid() && hasIdentifier();
}

//-----------//
// ValueDecl //
//-----------//

ValueDecl::ValueDecl(DeclKind kind, Identifier id, TypeLoc ty, bool isConst,
  SourceRange range):
  NamedDecl(kind, id, range), isConst_(isConst), type_(ty) {

}

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

bool ValueDecl::isValid() const {
  return NamedDecl::isValid() && type_;
}

//-----------//
// ParamDecl //
//-----------//

ParamDecl::ParamDecl():
  ParamDecl(Identifier(), TypeLoc(), false, SourceRange()) {

}

ParamDecl::ParamDecl(Identifier id, TypeLoc type, bool isConst, 
  SourceRange range):
  ValueDecl(DeclKind::ParamDecl, id, type, isConst, range) {

}

bool ParamDecl::isValid() const {
  return ValueDecl::isValid();
}

//----------//
// FuncDecl //
//----------//

FuncDecl::FuncDecl():
  FuncDecl(nullptr, Identifier(), nullptr, SourceRange(), SourceLoc()) {

}

FuncDecl::FuncDecl(TypeLoc returnType, Identifier fnId, CompoundStmt* body,
  SourceRange range, SourceLoc headerEndLoc):
  NamedDecl(DeclKind::FuncDecl, fnId, range), headEndLoc_(headerEndLoc), 
	body_(body), returnType_(returnType), 
  DeclContext(DeclContextKind::FuncDecl) { paramsAreValid_ = true; }

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

bool FuncDecl::isValid() const {
  return NamedDecl::isValid() && body_ && returnType_ && headEndLoc_ && paramsAreValid_;
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
  paramsAreValid_ = (param && param->isValid());

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

//---------//
// VarDecl //
//---------//

VarDecl::VarDecl():
  VarDecl(Identifier(), TypeLoc(), false, nullptr, SourceRange()) {

}

VarDecl::VarDecl(Identifier id, TypeLoc type, bool isConst, Expr* init, 
  SourceRange range):
  ValueDecl(DeclKind::VarDecl, id, type, isConst, range), init_(init) {

}

bool VarDecl::isValid() const {
  return ValueDecl::isValid();
}

Expr* VarDecl::getInitExpr() const {
  return init_;
}

bool VarDecl::hasInitExpr() const {
  return (bool)init_;
}

void VarDecl::setInitExpr(Expr* expr) {
  init_ = expr;
}

//----------//
// UnitDecl //
//----------//

UnitDecl::UnitDecl(Identifier id,FileID inFile): 
	NamedDecl(DeclKind::UnitDecl,id, SourceRange()), file_(inFile), 
  DeclContext(DeclContextKind::UnitDecl) { declsAreValid_ = true; }

void UnitDecl::addDecl(Decl* decl) {
  // Check the decl
  if (!decl->isValid())
    declsAreValid_ = false;

  // Update locs
  SourceRange range;
  if (!getRange().isValid()) /*(range not set yet) */ {
    assert((decls_.size() == 0) && "Range not set, but we already have decls?");
    range = decl->getRange();
  }
  else {
    assert((decls_.size() > 0) && "Range set, but we don't have decls?");
    range = SourceRange(
      getRange().getBegin(),
      decl->getRange().getEnd()
    );
  }
  assert(range && "Range is invalid");
  setRange(range);

  // Push it
  decls_.push_back(decl);
}

void UnitDecl::setDecl(Decl* decl, std::size_t idx) {
  assert(idx < decls_.size() && "out-of-range");
  decls_[idx] = decl;
}

Decl* UnitDecl::getDecl(std::size_t idx) const {
  assert(idx < decls_.size() && "out-of-range");
  return decls_[idx];
}

UnitDecl::DeclVecTy& UnitDecl::getDecls() {
  return decls_;
}

std::size_t UnitDecl::getNumDecls() const {
  return decls_.size();
}

bool UnitDecl::isValid() const {
  return file_ && NamedDecl::isValid() && (decls_.size() != 0) && declsAreValid_;
}

FileID UnitDecl::getFileID() const {
  return file_;
}

void UnitDecl::setFileID(const FileID& fid) {
  file_ = fid;
}