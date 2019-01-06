//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
// See LICENSE.txt for license info.            
// File : Decl.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/AST/Expr.hpp"
#include "Fox/AST/Decl.hpp"
#include "Fox/AST/Stmt.hpp"
#include "Fox/AST/Types.hpp"
#include "Fox/Common/Source.hpp"
#include "Fox/AST/ASTContext.hpp"
#include <sstream>
#include <cassert>

using namespace fox;

//----------------------------------------------------------------------------//
// Decl
//----------------------------------------------------------------------------//

#define DECL(ID, PARENT)\
  static_assert(std::is_trivially_destructible<ID>::value, \
  #ID " is allocated in the ASTContext: It's destructor is never called!");
#include "Fox/AST/DeclNodes.def"


Decl::Decl(DeclKind kind, Parent parent, SourceRange range):
  kind_(kind), range_(range), parent_(parent), 
  checkState_(CheckState::Unchecked) {
  assert((parent || isa<UnitDecl>(this)) && "Every decl except UnitDecls must"
    " have a parent!");
}

DeclKind Decl::getKind() const {
  return kind_;
}

DeclContext* Decl::getDeclContext() const {
  if(isParentNull()) return nullptr;
  if(DeclContext* ptr = parent_.dyn_cast<DeclContext*>())
    return ptr;
  return nullptr;
}

bool Decl::isLocal() const {
  return parent_.is<FuncDecl*>() && (!isParentNull());
}

FuncDecl* Decl::getFuncDecl() const {
  if(isParentNull()) 
    return nullptr;
  if(FuncDecl* ptr = parent_.dyn_cast<FuncDecl*>())
    return ptr;
  return nullptr;
}

Decl::Parent Decl::getParent() const {
  return parent_;
}

bool Decl::isParentNull() const {
  return parent_.isNull();
}

DeclContext* Decl::getClosestDeclContext() const {
  if(auto* dc = dyn_cast<DeclContext>(const_cast<Decl*>(this)))
    return dc;
  if(auto* fn = getFuncDecl())
    return fn->getDeclContext();
  return getDeclContext();
}

ASTContext& Decl::getASTContext() const {
  auto* closest = getClosestDeclContext();
  assert(closest && "should never return nullptr!");
  return closest->getASTContext();
}

void Decl::setRange(SourceRange range) {
  range_ = range;
}

SourceRange Decl::getRange() const {
  return range_;
}

SourceLoc Decl::getBegin() const {
  return range_.getBegin();
}

SourceLoc Decl::getEnd() const {
  return range_.getEnd();
}

bool Decl::isUnchecked() const {
  return (checkState_ == CheckState::Unchecked);
}

bool Decl::isChecking() const {
  return (checkState_ == CheckState::Checking);
}

bool Decl::isChecked() const {
  return (checkState_ == CheckState::Checked);
}

Decl::CheckState Decl::getCheckState() const {
  return checkState_;
}

void Decl::setCheckState(CheckState state) {
  checkState_ = state;
}

FileID Decl::getFileID() const {
  return range_.getBegin().getFileID();
}

void* Decl::operator new(std::size_t sz, ASTContext& ctxt, std::uint8_t align) {
  return ctxt.allocate(sz, align);
}

//----------------------------------------------------------------------------//
// NamedDecl
//----------------------------------------------------------------------------//

NamedDecl::NamedDecl(DeclKind kind, Parent parent, Identifier id, 
  SourceRange idRange, SourceRange range): Decl(kind, parent, range),
  identifier_(id), identifierRange_(idRange), illegalRedecl_(false){}

Identifier NamedDecl::getIdentifier() const {
  return identifier_;
}

void NamedDecl::setIdentifier(Identifier id, SourceRange idRange) {
  identifier_ = id;
  identifierRange_ = idRange;
}

bool NamedDecl::hasIdentifier() const {
  return !identifier_.isNull();
}

bool NamedDecl::isIllegalRedecl() const {
  return illegalRedecl_;
}

void NamedDecl::setIsIllegalRedecl(bool val) {
  illegalRedecl_ = val;
}

SourceRange NamedDecl::getIdentifierRange() const {
  return identifierRange_;
}

bool NamedDecl::hasIdentifierRange() const {
  return (bool)identifierRange_;
}

//----------------------------------------------------------------------------//
// ValueDecl
//----------------------------------------------------------------------------//

ValueDecl::ValueDecl(DeclKind kind, Parent parent, Identifier id, 
  SourceRange idRange, Type ty, SourceRange range): 
  NamedDecl(kind, parent, id, idRange, range), type_(ty) {}

Type ValueDecl::getType() const {
  return type_;
}

void ValueDecl::setType(Type ty) {
  type_ = ty;
}

bool ValueDecl::isConst() const {
  // Switch on the kind of this ValueDecl
  switch(getKind()) {
    case DeclKind::VarDecl:
      // VarDecls are const if they are declared using
      // "Let"
      return cast<VarDecl>(this)->isLet();
    case DeclKind::ParamDecl:
      // ParamDecls are constant if they aren't explicitely
      // mutable.
      return !(cast<ParamDecl>(this)->isMutable());
    case DeclKind::FuncDecl:
      // FuncDecls are always const.
      return true;
    default:
      fox_unreachable("Unknown ValueDecl kind!");
  }
}

//----------------------------------------------------------------------------//
// ParamDecl
//----------------------------------------------------------------------------//

ParamDecl* ParamDecl::create(ASTContext& ctxt, FuncDecl* parent, 
  Identifier id, SourceRange idRange, TypeLoc type, bool isMutable,
  SourceRange range) {
  return new(ctxt) ParamDecl(parent, id, idRange, type, isMutable, range);
}

bool ParamDecl::isMutable() const {
  return isMut_;
}

SourceRange ParamDecl::getTypeRange() const {
  return typeRange_;
}

void ParamDecl::setTypeRange(SourceRange range) {
  typeRange_ = range;
}

TypeLoc ParamDecl::getTypeLoc() const {
  return TypeLoc(getType(), getTypeRange());
}

ParamDecl::ParamDecl(FuncDecl* parent, Identifier id, SourceRange idRange, 
  TypeLoc type, bool isMutable, SourceRange range):
  ValueDecl(DeclKind::ParamDecl, parent, id, idRange, type.withoutLoc(), range),
  typeRange_(type.getRange()), isMut_(isMutable) {}

//----------------------------------------------------------------------------//
// ParamList
//----------------------------------------------------------------------------//

ParamList* ParamList::create(ASTContext& ctxt, ArrayRef<ParamDecl*> params) {
  auto totalSize = totalSizeToAlloc<ParamDecl*>(params.size());
  void* mem = ctxt.allocate(totalSize, alignof(ParamDecl));
  return new(mem) ParamList(params);
}

ArrayRef<ParamDecl*> ParamList::getArray() const {
  return {getTrailingObjects<ParamDecl*>(), numParams_};
}

MutableArrayRef<ParamDecl*> ParamList::getArray() {
  return {getTrailingObjects<ParamDecl*>(), numParams_};
}

ParamDecl*& ParamList::get(std::size_t idx) {
  assert((idx < numParams_) && "Out of range");
  return getArray()[idx];
}

const ParamDecl* ParamList::get(std::size_t idx) const {
  assert((idx < numParams_) && "Out of range");
  return getArray()[idx];
}
ParamList::SizeTy ParamList::getNumParams() const {
  return numParams_;
}

ParamList::iterator ParamList::begin() {
  return getArray().begin();
}

ParamList::iterator ParamList::end() {
  return getArray().end();
}

ParamList::const_iterator ParamList::begin() const {
  return getArray().begin();
}

ParamList::const_iterator ParamList::end() const {
  return getArray().end();
}

const ParamDecl* ParamList::operator[](std::size_t idx) const {
  return get(idx);
}

ParamDecl*& ParamList::operator[](std::size_t idx) {
  return get(idx);
}

ParamList::ParamList(ArrayRef<ParamDecl*> params) 
  : numParams_(static_cast<SizeTy>(params.size())) {
  assert((params.size() < maxParams) && "Too many parameters for ParamList. "
    "Change the type of SizeTy to something bigger!");
  std::uninitialized_copy(params.begin(), params.end(), 
    getTrailingObjects<ParamDecl*>());
}

void* ParamList::operator new(std::size_t, void* mem) {
  assert(mem);
  return mem;
}

//----------------------------------------------------------------------------//
// FuncDecl
//----------------------------------------------------------------------------//

FuncDecl::FuncDecl(DeclContext* parent, Identifier fnId, SourceRange idRange,
  TypeLoc returnType, SourceRange range):
  ValueDecl(DeclKind::FuncDecl, parent, fnId, idRange, Type(), range), 
  returnType_(returnType) {
  assert(returnType && "return type can't be null");
}

FuncDecl* FuncDecl::create(ASTContext& ctxt, DeclContext* parent, Identifier id,
  SourceRange idRange, TypeLoc returnType, SourceRange range) {
  return new(ctxt) FuncDecl(parent, id, idRange, returnType, range);
}

FuncDecl* FuncDecl::create(ASTContext& ctxt, DeclContext* parent) {
  TypeLoc voidTy(PrimitiveType::getVoid(ctxt));
  return create(ctxt, parent, Identifier(), SourceRange(), voidTy, 
    SourceRange());
}

void FuncDecl::setReturnTypeLoc(TypeLoc ty) {
  assert(ty && "return type can't be nullptr");
  returnType_ = ty;
  setType(Type());
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

bool FuncDecl::isReturnTypeImplicit() const {
  return !(getReturnTypeRange().isValid());
}

CompoundStmt* FuncDecl::getBody() const {
  return body_;
}

void FuncDecl::setParams(ParamList* params) {
  params_ = params;
  setType(Type());
}

ParamList* FuncDecl::getParams() {
  return params_;
}

bool FuncDecl::hasParams() const {
  return (bool)params_;
}

const ParamList* FuncDecl::getParams() const {
  return params_;
}

void FuncDecl::setBody(CompoundStmt* body) {
  body_ = body;
}

void FuncDecl::calculateType() {
  ASTContext& ctxt = getASTContext();
  assert(returnType_ && "return type can't be null!");
  // Collect the Parameter's type
  SmallVector<Type, 4> paramTys;
  for(ParamDecl* param : (*getParams())) {
    Type ty = param->getType();
    assert(ty && "param with null type!");
    paramTys.push_back(ty);
  }
  // Generate the FunctionType
  Type fn = FunctionType::get(ctxt, paramTys, returnType_.withoutLoc());
  setType(fn);
}

//----------------------------------------------------------------------------//
// VarDecl
//----------------------------------------------------------------------------//

VarDecl::VarDecl(Parent parent, Identifier id, SourceRange idRange, 
  TypeLoc type, Keyword kw, Expr* init, SourceRange range):
  ValueDecl(DeclKind::VarDecl, parent, id, idRange, type.withoutLoc(), range),
  initAndKW_(init, kw), typeRange_(type.getRange()) {}

VarDecl* VarDecl::create(ASTContext& ctxt, Parent parent, Identifier id,
  SourceRange idRange, TypeLoc type, Keyword kw, Expr* init, 
  SourceRange range) {
  return new(ctxt) VarDecl(parent, id, idRange, type, kw, init, range);
}

Expr* VarDecl::getInitExpr() const {
  return initAndKW_.getPointer();
}

bool VarDecl::hasInitExpr() const {
  return (bool)initAndKW_.getPointer();
}

bool VarDecl::isVar() const {
  // The int in initAndVarKind_ is set to false for Vars
  return (initAndKW_.getInt() == Keyword::Var);
}

bool VarDecl::isLet() const {
  return (initAndKW_.getInt() == Keyword::Let);
}

void VarDecl::setInitExpr(Expr* expr) {
  initAndKW_.setPointer(expr);
}

SourceRange VarDecl::getTypeRange() const {
  return typeRange_;
}

void VarDecl::setTypeRange(SourceRange range) {
  typeRange_ = range;
}

TypeLoc VarDecl::getTypeLoc() const {
  return TypeLoc(getType(), getTypeRange());
}

//----------------------------------------------------------------------------//
// UnitDecl
//----------------------------------------------------------------------------//

UnitDecl::UnitDecl(ASTContext& ctxt, Identifier id, FileID file):
  Decl(DeclKind::UnitDecl, (DeclContext*)nullptr, SourceRange()),
  identifier_(id), DeclContext(ctxt, DeclContextKind::UnitDecl),
  ctxt_(ctxt) {
  // Fetch the SourceRange from the SourceManager if the file is valid
  if(file) {
    SourceRange range = ctxt.sourceMgr.getRangeOfFile(file);
    assert(range && "getRangeOfFile returned an invalid range");
    setRange(range);
  }
}

UnitDecl* UnitDecl::create(ASTContext& ctxt,Identifier id, FileID file) {
  return new(ctxt) UnitDecl(ctxt, id, file);
}

Identifier UnitDecl::getIdentifier() const {
  return identifier_;
}

void UnitDecl::setIdentifier(Identifier id) {
  identifier_ = id;
}

ASTContext& UnitDecl::getASTContext() const {
  return ctxt_;
}
