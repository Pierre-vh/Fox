//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : Types.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/AST/Types.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/Common/LLVM.hpp"
#include "Fox/AST/ASTContext.hpp"
#include "Fox/AST/ASTVisitor.hpp"
#include "Fox/AST/ASTWalker.hpp"
#include <sstream>

using namespace fox;

namespace {
  class TypePrinter : public TypeVisitor<TypePrinter, void> {
    std::ostream& out;
    bool debugPrint = false;

    static constexpr char* nullTypeStr = "nullptr";
    static constexpr char* emptyCellTypeStr = "?";

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
          case PTK::FloatTy:
            out << "float";
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
          out << "@";
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
          if (Type elem = type->getSubstitution())
            visit(elem);
          else out << nullTypeStr;
          out << ")";
        }
        else {
          if(Type ty = type->getSubstitution())
            visit(ty);
          else 
            out << emptyCellTypeStr;
        }
      }
  };
}

//----------//
// TypeBase //
//----------//

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

Type TypeBase::getBoundRValue() {
  Type ty = getRValue()->deref();
  // Deref never returns a CellType unless
  // it's an unbound one.
  if(ty->is<CellType>())
    return nullptr;
  return ty;
}

namespace {
  Type derefImpl(TypeBase* type) {
    if (auto* cell = type->getAs<CellType>()) {
      Type sub = cell->getSubstitution();
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

bool TypeBase::isFloatType() const {
  if (auto* prim = dyn_cast<PrimitiveType>(this))
    return (prim->getPrimitiveKind() == PrimitiveType::Kind::FloatTy);
  return false;
}

bool TypeBase::isVoidType() const {
  if (auto* prim = dyn_cast<PrimitiveType>(this))
    return (prim->getPrimitiveKind() == PrimitiveType::Kind::VoidTy);
  return false;
}

void* TypeBase::operator new(size_t sz, ASTContext& ctxt, std::uint8_t align) {
  return ctxt.getAllocator().allocate(sz, align);
}

void TypeBase::calculateIsBound() const {
  class Impl : public ASTWalker {
    public:
      // If this method returns false, the walk is aborted.
      virtual bool handleTypePre(Type ty) override {
        if (auto* cell = ty->getAs<CellType>())
          return cell->hasSubstitution();
        return true;
      }
  };
  isBound_ = Impl().walk(const_cast<TypeBase*>(this));
  isBoundCalculated_ = true;
}

void TypeBase::initBitfields() {
  isBound_ = false;
  isBoundCalculated_ = false;
}

//-------------//
// BuiltinType //
//-------------//

BasicType::BasicType(TypeKind tc):
  TypeBase(tc) {

}

//---------------//
// PrimitiveType //
//---------------//

PrimitiveType::PrimitiveType(Kind kd)
  : builtinKind_(kd), BasicType(TypeKind::PrimitiveType) {

}

PrimitiveType* PrimitiveType::getString(ASTContext& ctxt) {
  if (!ctxt.theStringType)
    ctxt.theStringType = new(ctxt) PrimitiveType(Kind::StringTy);
  return ctxt.theStringType;
}

PrimitiveType* PrimitiveType::getChar(ASTContext& ctxt) {
  if (!ctxt.theCharType)
    ctxt.theCharType = new(ctxt) PrimitiveType(Kind::CharTy);
  return ctxt.theCharType;
}

PrimitiveType* PrimitiveType::getFloat(ASTContext& ctxt) {
  if (!ctxt.theFloatType)
    ctxt.theFloatType = new(ctxt) PrimitiveType(Kind::FloatTy);
  return ctxt.theFloatType;
}

PrimitiveType* PrimitiveType::getBool(ASTContext& ctxt) {
  if (!ctxt.theBoolType)
    ctxt.theBoolType = new(ctxt) PrimitiveType(Kind::BoolTy);
  return ctxt.theBoolType;
}

PrimitiveType* PrimitiveType::getInt(ASTContext& ctxt) {
  if (!ctxt.theIntType)
    ctxt.theIntType = new(ctxt) PrimitiveType(Kind::IntTy);
  return ctxt.theIntType;
}

PrimitiveType* PrimitiveType::getVoid(ASTContext& ctxt) {
  if (!ctxt.theVoidType)
    ctxt.theVoidType = new(ctxt) PrimitiveType(Kind::VoidTy);
  return ctxt.theVoidType;
}

PrimitiveType::Kind PrimitiveType::getPrimitiveKind() const {
  return builtinKind_;
}

//-----------//
// ArrayType //
//-----------//

ArrayType::ArrayType(Type elemTy):
  elementTy_(elemTy), TypeBase(TypeKind::ArrayType) {
  assert(elemTy && "The Array item type cannot be null!");
}

ArrayType* ArrayType::get(ASTContext& ctxt, Type ty) {
  auto lb = ctxt.arrayTypes.lower_bound(ty);
  if (lb != ctxt.arrayTypes.end() &&
    !(ctxt.lvalueTypes.key_comp()(ty, lb->first))) {
    // Key already exists, return lb->second.get()
    return lb->second;
  }
  else {
    // Key does not exists, insert & return.
    auto insertionResult = ctxt.arrayTypes.insert(lb, { ty , new(ctxt) ArrayType(ty) });
    return insertionResult->second;
  }
}

Type ArrayType::getElementType() {
  return elementTy_;
}

const Type ArrayType::getElementType() const {
  return elementTy_;
}

//------------//
// LValueType //
//------------//

LValueType::LValueType(Type type):
  TypeBase(TypeKind::LValueType), ty_(type) {
  assert(type && "cannot be null");
}

LValueType* LValueType::get(ASTContext& ctxt, Type ty) {
  auto lb = ctxt.lvalueTypes.lower_bound(ty);
  if (lb != ctxt.lvalueTypes.end() &&
    !(ctxt.lvalueTypes.key_comp()(ty, lb->first))) {
    // Key already exists, return lb->second.get()
    return lb->second;
  }
  else {
    // Key does not exists, insert & return.
    auto insertionResult = ctxt.lvalueTypes.insert(lb, { ty , new(ctxt) LValueType(ty) });
    return insertionResult->second;
  }
}

Type LValueType::getType() {
  return ty_;
}

const Type LValueType::getType() const {
  return ty_;
}

//-----------//
// ErrorType //
//-----------//

ErrorType::ErrorType():
  BasicType(TypeKind::ErrorType) {

}

ErrorType* ErrorType::get(ASTContext& ctxt) {
  if (!ctxt.theErrorType)
    ctxt.theErrorType = new(ctxt) ErrorType();
  return ctxt.theErrorType;
}

//----------//
// CellType //
//----------//

CellType::CellType(): TypeBase(TypeKind::CellType) {}

CellType* CellType::create(ASTContext& ctxt) {
  return new(ctxt) CellType();
}

Type CellType::getSubstitution() {
  return type_;
}

const Type CellType::getSubstitution() const {
  return type_;
}

bool CellType::hasSubstitution() const {
  return (type_ != nullptr);
}

void CellType::setSubstitution(Type type) {
  assert(type 
    && "Cannot set the substitution to a null pointer. Use reset() for that.");
  type_ = type;
}

void CellType::reset() {
  type_ = nullptr;
}

void* CellType::operator new(std::size_t sz, ASTContext& ctxt, std::uint8_t align) {
  return ctxt.getSemaAllocator().allocate(sz,align);
}
