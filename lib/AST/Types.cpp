//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
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
    static constexpr char emptyCellTypeStr[] = "any";

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

      void visitCellType(CellType* type) {
        if (debugPrint) {
          out << "Cell." << (void*)type << "(";
          if (Type elem = type->getSubst())
            visit(elem);
          else out << nullTypeStr;
          out << ")";
        }
        else {
          if(Type ty = type->getSubst())
            visit(ty);
          else 
            out << emptyCellTypeStr;
        }
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
  constexpr char TypePrinter::emptyCellTypeStr[];
}

//----------------------------------------------------------------------------//
// TypeBase
//----------------------------------------------------------------------------//

TypeBase::TypeBase(TypeKind tc):
  kind_(tc) {

}

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

bool TypeBase::isBound() const {
  if(!isBoundCalculated_)
    calculateIsBound();
  return isBound_;
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

Type TypeBase::getAsBoundRValue() {
  if (isBound()) {
    Type ty = getRValue()->deref();
    // Sanity check
    if(CellType* cell = ty->getAs<CellType>())
      assert(cell->hasSubst() 
      && "Type is bound but deref returned a unbound CellType?");
    return ty;
  }
  return nullptr;
}

namespace {
  Type derefImpl(TypeBase* type) {
    if (auto* cell = type->getAs<CellType>()) {
      Type sub = cell->getSubst();
      return sub ? derefImpl(sub.getPtr()) : type;
    }
    return type;
  }
}

Type TypeBase::deref() {
  return derefImpl(this);
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
      case Pk::BoolTy:
      case Pk::DoubleTy:
      case Pk::IntTy:
        return true;
      default:
        return false;
    }
  }
  return false;
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

void TypeBase::calculateIsBound() const {
  class Impl : public TypeWalker {
    public:
      // If this method returns false, the walk is aborted.
      virtual bool handleTypePre(Type ty) override {
        if (auto* cell = ty->getAs<CellType>())
          return cell->hasSubst();
        return true;
      }
  };
  bool result = Impl().walk(const_cast<TypeBase*>(this));
  setIsBound(result);
}

void TypeBase::setIsBound(bool val) const {
  isBoundCalculated_ = true;
  isBound_ = val;
}

void TypeBase::initBitfields() {
  isBound_ = false;
  isBoundCalculated_ = false;
}

//----------------------------------------------------------------------------//
// BasicType
//----------------------------------------------------------------------------//

BasicType::BasicType(TypeKind tc):
  TypeBase(tc) {

}

//----------------------------------------------------------------------------//
// PrimitiveType
//----------------------------------------------------------------------------//

PrimitiveType::PrimitiveType(Kind kd)
  : builtinKind_(kd), BasicType(TypeKind::PrimitiveType) {

}

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

}

ErrorType* ErrorType::get(ASTContext& ctxt) {
  if (!ctxt.theErrorType_)
    ctxt.theErrorType_ = new(ctxt) ErrorType();
  return ctxt.theErrorType_;
}

//----------------------------------------------------------------------------//
// CellType
//----------------------------------------------------------------------------//

CellType::CellType(): TypeBase(TypeKind::CellType) {}

CellType* CellType::create(ASTContext& ctxt) {
  return new(ctxt) CellType();
}

Type CellType::getSubst() const {
  return subst_;
}

bool CellType::hasSubst() const {
  return (subst_ != nullptr);
}

void CellType::setSubst(Type type) {
  assert(type 
    && "Cannot set the substitution to a null pointer. "
       "Use resetSubst() for that.");
  subst_ = type;
}

void CellType::resetSubst() {
  subst_ = nullptr;
}

void* CellType::operator new(std::size_t sz, ASTContext& ctxt,
  std::uint8_t align) {
  return ctxt.allocate(sz,align);
}

//----------------------------------------------------------------------------//
// ParamType
//----------------------------------------------------------------------------//

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
}