//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : Decl.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/AST/Expr.hpp"
#include "Fox/AST/Decl.hpp"
#include "Fox/AST/Stmt.hpp"
#include "Fox/AST/Types.hpp"
#include "Fox/AST/ASTContext.hpp"
#include "Fox/Common/Errors.hpp"

using namespace fox;

//----------------------------------------------------------------------------//
// Decl
//----------------------------------------------------------------------------//

#define DECL(ID, PARENT)\
  static_assert(std::is_trivially_destructible<ID>::value, \
  #ID " is allocated in the ASTContext: Its destructor is never called!");
#include "Fox/AST/DeclNodes.def"


Decl::Decl(DeclKind kind, DeclContext* dc): dc_(dc),  kind_(kind),
  checkState_(CheckState::Unchecked) {}

DeclKind Decl::getKind() const {
  return kind_;
}

DeclContext* Decl::getDeclContext() const {
  return dc_;
}

bool Decl::isLocal() const {
  return dc_ && isa<FuncDecl>(dc_);
}

DeclContext* Decl::getClosestDeclContext() const {
  if(auto* dc = dyn_cast<DeclContext>(const_cast<Decl*>(this)))
    return dc;
  return getDeclContext();
}

namespace {
  template<typename Rtr, typename Class>
  constexpr bool isOverridenFromDecl(Rtr (Class::*)() const) {
    return true;
  }

  template<typename Rtr>
  constexpr bool isOverridenFromDecl(Rtr (Decl::*)() const) {
    return false;
  }
}

SourceRange Decl::getSourceRange() const {
  switch(getKind()) {
    #define ASSERT_HAS_GETRANGE(ID)\
      static_assert(isOverridenFromDecl(&ID::getSourceRange),\
        #ID " does not reimplement getSourceRange()")
    #define DECL(ID, PARENT) case DeclKind::ID:\
      ASSERT_HAS_GETRANGE(ID); \
      return cast<ID>(this)->getSourceRange();
    #include "Fox/AST/DeclNodes.def"
    #undef ASSERT_HAS_GETRANGE
    default:
      fox_unreachable("all kinds handled");
  }
}

SourceLoc Decl::getBeginLoc() const {
  return getSourceRange().getBeginLoc();
}

SourceLoc Decl::getEndLoc() const {
  return getSourceRange().getEndLoc();
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
  return getSourceRange().getBeginLoc().getFileID();
}

void* Decl::operator new(std::size_t sz, ASTContext& ctxt, std::uint8_t align) {
  return ctxt.allocate(sz, align);
}

//----------------------------------------------------------------------------//
// NamedDecl
//----------------------------------------------------------------------------//

NamedDecl::NamedDecl(DeclKind kind, DeclContext* dc, Identifier id, 
                     SourceRange idRange): Decl(kind, dc),
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

ValueDecl::ValueDecl(DeclKind kind, DeclContext* dc, Identifier id, 
  SourceRange idRange):  NamedDecl(kind, dc, id, idRange) {}

namespace {
  template<typename Rtr, typename Class>
  constexpr bool isOverridenFromValueDecl(Rtr (Class::*)() const) {
    return true;
  }

  template<typename Rtr>
  constexpr bool isOverridenFromValueDecl(Rtr (Decl::*)() const) {
    return false;
  }
}
Type ValueDecl::getValueType() const {
  switch(getKind()) {
    #define ASSERT_HAS_GETVALUETYPE(ID)\
      static_assert(isOverridenFromValueDecl(&ID::getValueType),\
        #ID " does not reimplement getValueType()")
    #define VALUE_DECL(ID, PARENT) case DeclKind::ID:\
      ASSERT_HAS_GETVALUETYPE(ID); \
      return cast<ID>(this)->getValueType();
    #include "Fox/AST/DeclNodes.def"
    #undef ASSERT_HAS_GETVALUETYPE
    default:
      fox_unreachable("all kinds handled");
  }
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
      return !(cast<ParamDecl>(this)->isMut());
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

ParamDecl* ParamDecl::create(ASTContext& ctxt, DeclContext* dc, 
  Identifier id, SourceRange idRange, TypeLoc type, bool isMut) {
  return new(ctxt) ParamDecl(dc, id, idRange, type, isMut);
}

bool ParamDecl::isMut() const {
  return isMut_;
}

TypeLoc ParamDecl::getTypeLoc() const {
  return typeLoc_;
}

Type ParamDecl::getValueType() const {
  return typeLoc_.getType();
}

void ParamDecl::setIsUsed(bool value) {
  used_ = value;
}

bool ParamDecl::isUsed() const {
  return used_;
}

SourceRange ParamDecl::getSourceRange() const {
  return SourceRange(getIdentifierRange().getBeginLoc(), typeLoc_.getEndLoc());
}

ParamDecl::ParamDecl(DeclContext* dc, Identifier id, SourceRange idRange, 
  TypeLoc type, bool isMut):
  ValueDecl(DeclKind::ParamDecl, dc, id, idRange), typeLoc_(type), 
  isMut_(isMut), used_(false) {}

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

ParamDecl* ParamList::get(std::size_t idx) const {
  assert((idx < numParams_) && "Out of range");
  return getArray()[idx];
}

std::size_t ParamList::size() const {
  return numParams_;
}

ParamList::iterator ParamList::begin() const {
  return getArray().begin();
}

ParamList::iterator ParamList::end() const {
  return getArray().end();
}

ParamDecl* ParamList::operator[](std::size_t idx) const {
  return get(idx);
}

ParamList::ParamList(ArrayRef<ParamDecl*> params) 
  : numParams_(params.size()) {
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

FuncDecl::FuncDecl(DeclContext* parent, SourceLoc fnBegLoc, 
                    Identifier fnId, SourceRange idRange, ParamList* params, 
                    TypeLoc returnType): fnBegLoc_
  (fnBegLoc), ValueDecl(DeclKind::FuncDecl, parent, fnId, idRange),
  DeclContext(DeclContextKind::FuncDecl, parent), params_(params),
  returnTypeLoc_(returnType) {}

FuncDecl* 
FuncDecl::create(ASTContext& ctxt, DeclContext* parent, SourceLoc fnBegLoc,
  Identifier id, SourceRange idRange, ParamList* params, TypeLoc returnType) {
  return new(ctxt) 
    FuncDecl(parent, fnBegLoc, id, idRange, params, returnType);
}

void FuncDecl::setReturnTypeLoc(TypeLoc ty) {
  returnTypeLoc_ = ty;
}

TypeLoc FuncDecl::getReturnTypeLoc() const {
  return returnTypeLoc_;
}

bool FuncDecl::isReturnTypeImplicit() const {
  return !getReturnTypeLoc().isLocValid();
}

CompoundStmt* FuncDecl::getBody() const {
  return body_;
}

void FuncDecl::setParams(ParamList* params) {
  params_ = params;
}

bool FuncDecl::hasParams() const {
  return (bool)params_;
}

std::size_t FuncDecl::numParams() const {
  return params_ ? params_->size() : 0;
}

std::size_t FuncDecl::numUsedParams() const {
  if (params_) {
    std::size_t tot = 0;
    for (ParamDecl* param : *params_) 
      if(param->isUsed()) ++tot;
    return tot;
  }
  return 0;
}

Type FuncDecl::getValueType() const {
  return valueType_;
}

ParamList* FuncDecl::getParams() const {
  return params_;
}

void FuncDecl::setBody(CompoundStmt* body) {
  body_ = body;
}

void FuncDecl::calculateValueType() {
  ASTContext& ctxt = getASTContext();
  assert(returnTypeLoc_.isTypeValid() && "ill-formed FuncDecl: "
    "no return type");
  // Collect the param types
  SmallVector<FunctionTypeParam, 4> paramTypes;
  if (ParamList* params = getParams()) {
    for(ParamDecl* param : *params) {
      Type ty = param->getValueType();
      bool isMut = param->isMut();
      assert(ty && "ill-formed FuncDecl: Parameter with null type");
      paramTypes.emplace_back(ty, isMut);
    }
  }
  // Generate the FunctionType
  valueType_ = FunctionType::get(ctxt, paramTypes, returnTypeLoc_.getType());
}

SourceRange FuncDecl::getSourceRange() const {
  assert(body_ && "ill formed FuncDecl");
  return SourceRange(fnBegLoc_, body_->getEndLoc());
}

//----------------------------------------------------------------------------//
// BuiltinFuncDecl
//----------------------------------------------------------------------------//

BuiltinFuncDecl* BuiltinFuncDecl::get(ASTContext&, BuiltinID) {
  // TODO
  return nullptr;
}

SourceRange BuiltinFuncDecl::getSourceRange() const {
  // No SourceRange available since this is implicit.
  return SourceRange();
}

Type BuiltinFuncDecl::getValueType() const {
  // TODO
  return Type();
}

BuiltinID BuiltinFuncDecl::getBuiltinID() const {
  return bID_;
}

BuiltinFuncDecl::BuiltinFuncDecl(ASTContext& ctxt, DeclContext* dc, 
                                 BuiltinID id) : 
  ValueDecl(DeclKind::BuiltinFuncDecl, dc, 
            ctxt.getIdentifier(id), SourceRange()) {}

//----------------------------------------------------------------------------//
// VarDecl
//----------------------------------------------------------------------------//

VarDecl::VarDecl(DeclContext* dc, Identifier id, SourceRange idRange, 
  TypeLoc type, Keyword kw, Expr* init, SourceRange range):
  ValueDecl(DeclKind::VarDecl, dc, id, idRange), typeLoc_(type),
  range_(range), initAndKW_(init, kw) {}

VarDecl* VarDecl::create(ASTContext& ctxt, DeclContext* dc, Identifier id,
  SourceRange idRange, TypeLoc type, Keyword kw, Expr* init, 
  SourceRange range) {
  return new(ctxt) VarDecl(dc, id, idRange, type, kw, init, range);
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

TypeLoc VarDecl::getTypeLoc() const {
  return typeLoc_;
}

Type VarDecl::getValueType() const {
  return typeLoc_.getType();
}

SourceRange VarDecl::getSourceRange() const {
  return range_;
}

//----------------------------------------------------------------------------//
// UnitDecl
//----------------------------------------------------------------------------//

UnitDecl::UnitDecl(ASTContext& ctxt, Identifier id, FileID file):
  Decl(DeclKind::UnitDecl, (DeclContext*)nullptr), identifier_(id), file_(file),
  ctxt_(ctxt), DeclContext(DeclContextKind::UnitDecl, nullptr) {}

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

SourceRange UnitDecl::getSourceRange() const {
  if (Decl* first = getFirstDecl()) {
    Decl* last = getLastDecl();
    SourceRange range(first->getBeginLoc(), last->getEndLoc());
    assert(range.getFileID() == file_ && "broken UnitDecl");
    return range;
  }
  return SourceRange();
}