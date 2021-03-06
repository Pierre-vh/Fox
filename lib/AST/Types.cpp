//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : Types.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/AST/Types.hpp"
#include "Fox/AST/Type.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/AST/ASTContext.hpp"
#include "Fox/AST/ASTVisitor.hpp"
#include "Fox/AST/TypeVisitor.hpp"
#include "Fox/AST/ASTWalker.hpp"
#include "llvm/ADT/Hashing.h"
#include <sstream>

using namespace fox;

#define TYPE(ID, PARENT)\
  static_assert(std::is_trivially_destructible<ID>::value, \
  #ID " is allocated in the ASTContext: Its destructor is never called!");
#include "Fox/AST/TypeNodes.def"

//----------------------------------------------------------------------------//
// Type
//----------------------------------------------------------------------------//

Type::Type(TypeBase* ty) :
  ty_(ty) {}

TypeBase* Type::getPtr() {
  return ty_;
}

const TypeBase* Type::getPtr() const {
  return ty_;
}

bool Type::isNull() const {
  return (ty_ == nullptr);
}

TypeBase* Type::operator->() {
  assert(ty_ && "Cannot use this if the pointer is null.");
  return ty_;
}

const TypeBase* Type::operator->() const {
  assert(ty_ && "Cannot use this if the pointer is null.");
  return ty_;
}

Type::operator bool() const {
  return !isNull();
}

bool Type::operator==(const Type& type) const {
  return ty_ == type.ty_;
}

bool Type::operator!=(const Type& type) const {
  return !(*this == type);
}

bool Type::operator<(const Type other) const {
  return getPtr() < other.getPtr();
}

std::ostream& fox::operator<<(std::ostream& os, Type ty) {
  assert(ty && "can't print a null type");
  os << ty->to_string();
  return os;
}

//----------------------------------------------------------------------------//
// TypeLoc
//----------------------------------------------------------------------------//

TypeLoc::TypeLoc(Type ty, SourceRange range) :
  ty_(ty), range_(range) {}

SourceRange TypeLoc::getSourceRange() const {
  return range_;
}

SourceLoc TypeLoc::getBeginLoc() const {
  return range_.getBeginLoc();
}

SourceLoc TypeLoc::getEndLoc() const {
  return range_.getEndLoc();
}

bool TypeLoc::isLocValid() const {
  return (bool)range_;
}

bool TypeLoc::isTypeValid() const {
  return (bool)ty_;
}

bool TypeLoc::isComplete() const {
  return isLocValid() && isTypeValid();
}

Type TypeLoc::getType() {
  return ty_;
}

const Type TypeLoc::getType() const {
  return ty_;
}

//----------------------------------------------------------------------------//
// TypePrinter
//----------------------------------------------------------------------------//

namespace {
  class TypePrinter : public TypeVisitor<TypePrinter, void> {
    std::ostream& out;
    bool debugPrint;
    bool onlyPrintKind;

    public:
      TypePrinter(std::ostream& out, bool debugPrint, bool onlyPrintKind) :
        out(out), debugPrint(debugPrint), onlyPrintKind(onlyPrintKind) {}

      #define LEAF(TYPE, STR) void visit##TYPE(TYPE*) { out << STR; }
      LEAF(ErrorType,   "<error>")
      LEAF(IntegerType, "int")
      LEAF(DoubleType,  "double")
      LEAF(BoolType,    "bool")
      LEAF(CharType,    "char")
      LEAF(StringType,  "string")
      LEAF(VoidType,    "void")
      #undef LEAF

      void visitArrayType(ArrayType* type) {
        if (onlyPrintKind) {
          out << "array";
          return;
        }

        out << "[";
        visit(type->getElementType());
        out << "]";
      }

      void visitLValueType(LValueType* type) {
        if (debugPrint) {
          out << "@(";
          visit(type->getType());
          out << ")";
        }
        else
          visit(type->getType());
      }

      void visitTypeVariableType(TypeVariableType* type) {
        if (debugPrint) {
          out << "$T" << type->getNumber();
          // In debug prints, print the substitution too, if
          // there's one.
          if (Type subst = type->getSubst()) {
            out << "=(";
            visit(subst);
            out << ")";
          }
        }
        // When presenting this type to the user, just say it's a "any"
        // type.
        else 
          out << "any";
      }

      void visitFunctionType(FunctionType* type) {
        out << "(";
        bool first = true;
        for(auto param : type->getParams()) {
          // Print a colon for every type except the first one
          if(first) 
            first = false;
          else 
            out << ", ";
          // Print the type
          visit(param);
        }
        out << ") -> ";
        if(Type rtr = type->getReturnType())
          visit(rtr);
      }
  };
}

//----------------------------------------------------------------------------//
// TypeBase
//----------------------------------------------------------------------------//

TypeBase::TypeBase(TypeKind tc): kind_(tc) {
  propsValue_ = 0;
}

std::string TypeBase::to_string() const {
  std::ostringstream oss;
  TypePrinter tp(oss, /* debugPrint */ false, /*printOnlyKind*/ false);
  tp.visit(const_cast<TypeBase*>(this));
  return oss.str();
}

std::string TypeBase::getTypeFamilyString() const {
  std::ostringstream oss;
  TypePrinter tp(oss, /* debugPrint */ false, /*printOnlyKind*/ true);
  tp.visit(const_cast<TypeBase*>(this));
  return oss.str();
}

std::string TypeBase::toDebugString() const {
  std::ostringstream oss;
  TypePrinter tp(oss, /* debugPrint */ true, /*printOnlyKind*/ false);
  tp.visit(const_cast<TypeBase*>(this));
  return oss.str();
}

TypeKind TypeBase::getKind() const {
  return kind_;
}

Type TypeBase::getRValue() {
  if (LValueType* tmp = dyn_cast<LValueType>(this))
    return tmp->getType();
  return this;
}

const Type TypeBase::getRValue() const {
  return const_cast<TypeBase*>(this)->getRValue();
}

bool TypeBase::hasTypeVariable() const {
  return getProperties() & Property::HasTypeVariable;
}

bool TypeBase::hasErrorType() const {
  return getProperties() & Property::HasErrorType;
}

bool TypeBase::isStringType() const {
  return getRValue()->is<StringType>();
}

bool TypeBase::isCharType() const {
  return getRValue()->is<CharType>();
}

bool TypeBase::isBoolType() const {
  return getRValue()->is<BoolType>();
}

bool TypeBase::isIntType() const {
  return getRValue()->is<IntegerType>();
}

bool TypeBase::isDoubleType() const {
  return getRValue()->is<DoubleType>();
}

bool TypeBase::isVoidType() const {
  return getRValue()->is<VoidType>();
}

bool TypeBase::isNumericType() const {
  auto rvalue = getRValue();
  return rvalue->is<IntegerType>() || rvalue->is<DoubleType>();
}

bool TypeBase::isArrayType() const {
  return getRValue()->is<ArrayType>();
}

bool TypeBase::isReferenceType() const {
  if(auto rvalue = getRValue())
    return rvalue->is<ArrayType>() || rvalue->is<StringType>();
  return false;
}

bool TypeBase::isPrimitiveType() const {
  return getRValue()->is<PrimitiveType>();
}

bool TypeBase::isNumericOrBool() const {
  auto rvalue = getRValue();
  return rvalue->is<IntegerType>()    || rvalue->is<DoubleType>()
      || rvalue->is<BoolType>();
}

bool TypeBase::isAssignable() const {
  return this->is<LValueType>();
}

void* TypeBase::operator new(size_t sz, ASTContext& ctxt, 
  std::uint8_t align) {
  return ctxt.allocate(sz, align);
}

void* TypeBase::operator new(std::size_t, void* buff) {
  assert(buff);
  return buff;
}

TypeBase::Properties TypeBase::getProperties() const {
  return Properties(propsValue_);
}

void TypeBase::setProperties(Properties props) {
  assert((propsValue_ == 0) && "Properties have already been initialized!");
  propsValue_ = props.getValue();
  assert(propsValue_ == props.getValue() && "bits dropped");
}

//----------------------------------------------------------------------------//
// BasicType
//----------------------------------------------------------------------------//

BasicType::BasicType(TypeKind tc): TypeBase(tc) {}

//----------------------------------------------------------------------------//
// PrimitiveType
//----------------------------------------------------------------------------//

PrimitiveType::PrimitiveType(TypeKind tc): BasicType(tc) {}

//----------------------------------------------------------------------------//
// IntegerType
//----------------------------------------------------------------------------//

IntegerType* IntegerType::get(ASTContext& ctxt) {
  if (!ctxt.theIntType_)
    ctxt.theIntType_ = new(ctxt) IntegerType();
  return ctxt.theIntType_;
}

IntegerType::IntegerType() : PrimitiveType(TypeKind::IntegerType) {}

//----------------------------------------------------------------------------//
// DoubleType
//----------------------------------------------------------------------------//

DoubleType* DoubleType::get(ASTContext& ctxt) {
  if (!ctxt.theDoubleType)
    ctxt.theDoubleType = new(ctxt) DoubleType();
  return ctxt.theDoubleType;
}

DoubleType::DoubleType() : PrimitiveType(TypeKind::DoubleType) {}

//----------------------------------------------------------------------------//
// CharType
//----------------------------------------------------------------------------//

CharType* CharType::get(ASTContext& ctxt) {
  if (!ctxt.theCharType_)
    ctxt.theCharType_ = new(ctxt) CharType();
  return ctxt.theCharType_;
}

CharType::CharType() : PrimitiveType(TypeKind::CharType) {}

//----------------------------------------------------------------------------//
// BoolType
//----------------------------------------------------------------------------//

BoolType* BoolType::get(ASTContext& ctxt) {
  if (!ctxt.theBoolType_)
    ctxt.theBoolType_ = new(ctxt) BoolType();
  return ctxt.theBoolType_;
}

BoolType::BoolType() : PrimitiveType(TypeKind::BoolType) {}

//----------------------------------------------------------------------------//
// StringType
//----------------------------------------------------------------------------//

StringType* StringType::get(ASTContext& ctxt) {
  if (!ctxt.theStringType_)
    ctxt.theStringType_ = new(ctxt) StringType();
  return ctxt.theStringType_;
}

StringType::StringType() : PrimitiveType(TypeKind::StringType) {}

//----------------------------------------------------------------------------//
// VoidType
//----------------------------------------------------------------------------//

VoidType* VoidType::get(ASTContext& ctxt) {
  if (!ctxt.theVoidType_)
    ctxt.theVoidType_ = new(ctxt) VoidType();
  return ctxt.theVoidType_;
}

VoidType::VoidType() : PrimitiveType(TypeKind::VoidType) {}

//----------------------------------------------------------------------------//
// ErrorType
//----------------------------------------------------------------------------//

ErrorType::ErrorType():
  BasicType(TypeKind::ErrorType) {
  setProperties(Property::HasErrorType);
}

ErrorType* ErrorType::get(ASTContext& ctxt) {
  if (!ctxt.theErrorType_)
    ctxt.theErrorType_ = new(ctxt) ErrorType();
  return ctxt.theErrorType_;
}

//----------------------------------------------------------------------------//
// ArrayType
//----------------------------------------------------------------------------//

ArrayType::ArrayType(Type elemTy):
  elementTy_(elemTy), TypeBase(TypeKind::ArrayType) {
  assert(elemTy && "cannot be null");
  setProperties(elemTy->getProperties());
}

ArrayType* ArrayType::get(ASTContext& ctxt, Type ty) {
  TypeBase* ptr = ty.getPtr();
  auto lb = ctxt.arrayTypes_.lower_bound(ptr);
  if (lb != ctxt.arrayTypes_.end() &&
    !(ctxt.lvalueTypes_.key_comp()(ptr, lb->first))) {
    // Key already exists, return lb->second.get()
    return lb->second;
  }
  else {
    // Key does not exists, insert & return.
    auto insertionResult = ctxt.arrayTypes_.insert(lb, 
      {ptr , new(ctxt) ArrayType(ty) }
    );
    return insertionResult->second;
  }
}

Type ArrayType::getElementType() const {
  return elementTy_;
}

//----------------------------------------------------------------------------//
// LValueType
//----------------------------------------------------------------------------//

LValueType::LValueType(Type type):
  TypeBase(TypeKind::LValueType), ty_(type) {
  assert(type && "cannot be null");
  assert((!type->is<LValueType>()) && "Can't create nested LValueTypes!");
  setProperties(type->getProperties());
}

LValueType* LValueType::get(ASTContext& ctxt, Type ty) {
  TypeBase* ptr = ty.getPtr();
  auto lb = ctxt.lvalueTypes_.lower_bound(ptr);
  if (lb != ctxt.lvalueTypes_.end() &&
    !(ctxt.lvalueTypes_.key_comp()(ptr, lb->first))) {
    // Key already exists, return lb->second.get()
    return lb->second;
  }
  else {
    // Key does not exists, insert & return.
    auto insertionResult = ctxt.lvalueTypes_.insert(lb, 
      { ptr , new(ctxt) LValueType(ty) }
    );
    return insertionResult->second;
  }
}

Type LValueType::getType() const {
  return ty_;
}

//----------------------------------------------------------------------------//
// FunctionType
//----------------------------------------------------------------------------//

namespace {
  std::size_t hashFunctionType(ArrayRef<Type> paramTys, Type rtrTy) {
    // Collect types
    SmallVector<TypeBase*, 8> types;
    types.push_back(rtrTy.getPtr());
    for(auto param : paramTys) 
      types.push_back(param.getPtr());
    // Hash
    return llvm::hash_combine_range(types.begin(), types.end());
  }
}

FunctionType* 
FunctionType::get(ASTContext& ctxt, ArrayRef<Type> params, Type rtr) {
  // Hash the parameters.
  std::size_t hash = hashFunctionType(params, rtr);
  // Check in the map
  auto& map = ctxt.functionTypes_;
  auto it = map.find(hash);
  if(it != map.end()) {
    // This signature already exists in the map.
    FunctionType* fn = it->second;
    // Sanity check : compare that they're strictly equal. If they're
    // not, we may have encountered a hash collision.
    if(!fn->isSame(params, rtr)) {
      fox_unreachable("Hash collision detected. Two different function types "
        "had the same hash value!");
    }

    return fn;
  } 
  else {
    // It's the first time we've seen this signature, create a new
    // instance of FunctionType and insert it in the map.
    auto totalSize = totalSizeToAlloc<Type>(params.size());
    void* mem = ctxt.allocate(totalSize, alignof(FunctionType));
    FunctionType* created =  new(mem) FunctionType(params, rtr);
    map.insert({hash, created});
    return created;
  }
}

bool FunctionType::isSame(ArrayRef<Type> params, Type rtr) {
  Type myRtrTy = getReturnType();

  // Check that the return type matches
  if(myRtrTy != rtr) return false;

  // Check that the number of parameters matches
  if(params.size() != numParams()) return false;

  // Check parameters individually
  std::size_t num = numParams();
  for(std::size_t idx = 0; idx < num; ++idx) {
    Type param = params[idx];
    Type myParam = getParam(idx);
    if(param != myParam) return false;
  }
  return true;
}

Type FunctionType::getReturnType() const {
  return rtrType_;
}

ArrayRef<Type> FunctionType::getParams() const {
  return {getTrailingObjects<Type>(), numParams_};
}

Type FunctionType::getParam(std::size_t idx) const {
  assert((idx < numParams_) && "Out of range");
  return getParams()[idx];
}

FunctionType::SizeTy FunctionType::numParams() const {
  return numParams_;
}

FunctionType::FunctionType(ArrayRef<Type> params, Type rtr) :
  TypeBase(TypeKind::FunctionType), rtrType_(rtr),
  numParams_(static_cast<SizeTy>(params.size())) {
  assert((params.size() < maxParams) && "Too many params for FunctionType. "
    "Change the type of SizeTy to something bigger!");

  std::uninitialized_copy(params.begin(), params.end(),
    getTrailingObjects<Type>());

  setProperties(getPropertiesForFunc(params, rtr));
}

TypeBase::Properties 
FunctionType::getPropertiesForFunc(ArrayRef<Type> params, Type rtr) {
  Properties props;
  props |= rtr->getProperties();
  for(auto param : params) 
    props |= param->getProperties();
  return props;
}

//----------------------------------------------------------------------------//
// TypeVariableType
//----------------------------------------------------------------------------//

TypeVariableType*
TypeVariableType::create(ASTContext& ctxt, std::uint16_t number) {
  return new(ctxt) TypeVariableType(number);
}

std::uint16_t TypeVariableType::getNumber() const {
  return number_;
}

Type TypeVariableType::getSubst() const {
  return subst_;
}

bool TypeVariableType::hasSubst() const {
  return (bool)subst_;
}

Type TypeVariableType::getSubstRecursively() const {
  Type subst = getSubst();
  // If the subst is non-null and it's a TypeVariable, return
  // subst->getSubstRecursively()
  if(subst && subst->is<TypeVariableType>())
    return subst->getAs<TypeVariableType>()->getSubstRecursively();
  return subst;
}

void TypeVariableType::assignSubst(Type type) {
  assert(subst_.isNull() && "This type already have a"
    " substitution!");
  subst_ = type;
}

TypeVariableType::TypeVariableType(std::uint16_t number): 
  TypeBase(TypeKind::TypeVariableType), number_(number) {
  setProperties(Property::HasTypeVariable);
}