//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
// See LICENSE.txt for license info.            
// File : Types.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/AST/Types.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/AST/ASTContext.hpp"
#include "Fox/AST/ASTVisitor.hpp"
#include "Fox/AST/ASTWalker.hpp"
#include "llvm/ADT/Hashing.h"
#include <sstream>

using namespace fox;

#define TYPE(ID, PARENT)\
  static_assert(std::is_trivially_destructible<ID>::value, \
  #ID " is allocated in the ASTContext: It's destructor is never called!");
#include "Fox/AST/TypeNodes.def"

//----------------------------------------------------------------------------//
// TypePrinter
//----------------------------------------------------------------------------//

namespace {
  class TypePrinter : public TypeVisitor<TypePrinter, void> {
    std::ostream& out;
    bool debugPrint = false;

    static constexpr char nullTypeStr[] = "nullptr";

    public:
      TypePrinter(std::ostream& out, bool debugPrint) :
        out(out), debugPrint(debugPrint) {

      }

      void visitPrimitiveType(PrimitiveType* type) {
        using PTK = PrimitiveType::Kind;
        switch (type->getPrimitiveKind()) {
          case PTK::BoolTy:
            out << "bool";
            break;
          case PTK::CharTy:
            out << "char";
            break;
          case PTK::StringTy:
            out << "string";
            break;
          case PTK::DoubleTy:
            out << "double";
            break;
          case PTK::IntTy:
            out << "int";
            break;
          case PTK::VoidTy:
            out << "void";
            break;
          default:
            fox_unreachable("all primitive kinds handled");
        }
      }

      void visitArrayType(ArrayType* type) {
        if (debugPrint) {
          out << "Array(";
          if (Type elem = type->getElementType())
            visit(elem);
          else
            out << nullTypeStr;
          out << ")";
        }
        else {
          out << "[";
          if (Type elem = type->getElementType())
            visit(elem);
          else
            out << nullTypeStr;
          out << "]";
        }
      }

      void visitLValueType(LValueType* type) {
        if (debugPrint) {
          out << "LValue(";
          if (Type elem = type->getType())
            visit(elem);
          else
            out << nullTypeStr;
          out << ")";
        }
        else {
          //out << "@";
          if (Type elem = type->getType())
            visit(elem);
          else
            out << nullTypeStr;
        }
      }

      void visitErrorType(ErrorType*) {
        out << "<error_type>";
      }

      void visitTypeVariableType(TypeVariableType* type) {
        if (debugPrint) {
          out << "$T" << type->getNumber();
          // In debug prints, print the substitution too, if
          // there's one.
          if (Type subst = type->getSubst()) {
            out << "=";
            visit(subst);
          }
        }
        else 
          out << "any";
      }

      void visitFunctionType(FunctionType* type) {
        out << "(";
        bool first = true;
        for(auto paramTy : type->getParamTypes()) {
          // Print a colon for every type except the first one
          if(first) first = false;
          else out << ",";
          // Print the type of the parameter
          if(paramTy)
            visit(paramTy);
          else out << nullTypeStr;
        }
        out << ") -> ";
        if(Type rtr = type->getReturnType())
          visit(rtr);
      }
  };

  constexpr char TypePrinter::nullTypeStr[];
}

//----------------------------------------------------------------------------//
// TypeBase
//----------------------------------------------------------------------------//

TypeBase::TypeBase(TypeKind tc): kind_(tc) {}

std::string TypeBase::toString() const {
  std::ostringstream oss;
  TypePrinter tp(oss, /* debug print */ false);
  // This is ugly but needed. TypePrinter won't alter
  // this instance anyway so it's meaningless.
  tp.visit(const_cast<TypeBase*>(this));
  return oss.str();
}

std::string TypeBase::toDebugString() const {
  std::ostringstream oss;
  TypePrinter tp(oss, /* debug print */ true);
  // See above (lines 128/129)
  tp.visit(const_cast<TypeBase*>(this));
  return oss.str();
}

TypeKind TypeBase::getKind() const {
  return kind_;
}

Type TypeBase::unwrapIfArray() {
  if (ArrayType* tmp = dyn_cast<ArrayType>(this))
    return tmp->getElementType();
  return nullptr;
}

Type TypeBase::getRValue() {
  if (LValueType* tmp = dyn_cast<LValueType>(this))
    return tmp->getType();
  return this;
}

bool TypeBase::hasTypeVariable() const {
  return getProperties() & Property::HasTypeVariable;
}

bool TypeBase::hasErrorType() const {
  return getProperties() & Property::HasErrorType;
}

bool TypeBase::isStringType() const {
  if(auto* prim = dyn_cast<PrimitiveType>(this))
    return (prim->getPrimitiveKind() == PrimitiveType::Kind::StringTy);
  return false;
}

bool TypeBase::isCharType() const {
  if (auto* prim = dyn_cast<PrimitiveType>(this))
    return (prim->getPrimitiveKind() == PrimitiveType::Kind::CharTy);
  return false;
}

bool TypeBase::isBoolType() const {
  if (auto* prim = dyn_cast<PrimitiveType>(this))
    return (prim->getPrimitiveKind() == PrimitiveType::Kind::BoolTy);
  return false;
}

bool TypeBase::isIntType() const {
  if (auto* prim = dyn_cast<PrimitiveType>(this))
    return (prim->getPrimitiveKind() == PrimitiveType::Kind::IntTy);
  return false;
}

bool TypeBase::isDoubleType() const {
  if (auto* prim = dyn_cast<PrimitiveType>(this))
    return (prim->getPrimitiveKind() == PrimitiveType::Kind::DoubleTy);
  return false;
}

bool TypeBase::isVoidType() const {
  if (auto* prim = dyn_cast<PrimitiveType>(this))
    return (prim->getPrimitiveKind() == PrimitiveType::Kind::VoidTy);
  return false;
}

bool TypeBase::isNumeric() const {
  if (auto* prim = getAs<PrimitiveType>()) {
    using Pk = PrimitiveType::Kind;
    switch (prim->getPrimitiveKind()) {
      case Pk::DoubleTy:
      case Pk::IntTy:
        return true;
      default:
        return false;
    }
  }
  return false;
}

bool TypeBase::isNumericOrBool() const {
  return isNumeric() || isBoolType();
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

PrimitiveType::PrimitiveType(Kind kd) : builtinKind_(kd), 
  BasicType(TypeKind::PrimitiveType) {}

PrimitiveType* PrimitiveType::getString(ASTContext& ctxt) {
  if (!ctxt.theStringType_)
    ctxt.theStringType_ = new(ctxt) PrimitiveType(Kind::StringTy);
  return ctxt.theStringType_;
}

PrimitiveType* PrimitiveType::getChar(ASTContext& ctxt) {
  if (!ctxt.theCharType_)
    ctxt.theCharType_ = new(ctxt) PrimitiveType(Kind::CharTy);
  return ctxt.theCharType_;
}

PrimitiveType* PrimitiveType::getDouble(ASTContext& ctxt) {
  if (!ctxt.theFloatType_)
    ctxt.theFloatType_ = new(ctxt) PrimitiveType(Kind::DoubleTy);
  return ctxt.theFloatType_;
}

PrimitiveType* PrimitiveType::getBool(ASTContext& ctxt) {
  if (!ctxt.theBoolType_)
    ctxt.theBoolType_ = new(ctxt) PrimitiveType(Kind::BoolTy);
  return ctxt.theBoolType_;
}

PrimitiveType* PrimitiveType::getInt(ASTContext& ctxt) {
  if (!ctxt.theIntType_)
    ctxt.theIntType_ = new(ctxt) PrimitiveType(Kind::IntTy);
  return ctxt.theIntType_;
}

PrimitiveType* PrimitiveType::getVoid(ASTContext& ctxt) {
  if (!ctxt.theVoidType_)
    ctxt.theVoidType_ = new(ctxt) PrimitiveType(Kind::VoidTy);
  return ctxt.theVoidType_;
}

PrimitiveType::Kind PrimitiveType::getPrimitiveKind() const {
  return builtinKind_;
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
// FunctionType
//----------------------------------------------------------------------------//

namespace {
  std::size_t functionTypeHash(ArrayRef<Type> paramTys, Type rtrTy) {
    // Create a vector of uintptr_t with all of the hash data
    SmallVector<std::uintptr_t, 8> bytes;
    // Return type pointer
    bytes.push_back(reinterpret_cast<std::uintptr_t>(rtrTy.getPtr()));
    // Param types pointers
    for(auto ty : paramTys) 
      bytes.push_back(reinterpret_cast<std::uintptr_t>(ty.getPtr()));
    // hash the data
    return llvm::hash_combine_range(bytes.begin(), bytes.end());
  }
}

FunctionType* FunctionType::get(ASTContext& ctxt, ArrayRef<Type> params, 
  Type rtr) {
  // Hash the parameters.
  std::size_t hash = functionTypeHash(params, rtr);
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
    Type myParam = getParamType(idx);
    if(param != myParam) return false;
  }
  return true;
}

Type FunctionType::getReturnType() const {
  return rtrType_;
}

ArrayRef<Type> FunctionType::getParamTypes() const {
  return {getTrailingObjects<Type>(), numParams_};
}

Type FunctionType::getParamType(std::size_t idx) const {
  assert((idx < numParams_) && "Out of range");
  return getParamTypes()[idx];
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

  SmallVector<Type, 8> containedTypes(params.begin(), params.end());
  containedTypes.push_back(rtr);
  assert(containedTypes.size() == (params.size()+1));

  // Set the properties
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
