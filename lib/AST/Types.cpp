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
#include <sstream>

using namespace fox;

namespace {
  class TypePrinter : public TypeVisitor<TypePrinter, void> {
    std::ostream& out;
    bool debugPrint = false;

    static constexpr char * nullTypeStr = "<none>";

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
          if (TypeBase* elem = type->getElementType())
            visit(elem);
          else
            out << nullTypeStr;
          out << ")";
        }
        else {
          out << "[";
          if (TypeBase* elem = type->getElementType())
            visit(elem);
          else
            out << nullTypeStr;
          out << "]";
        }
      }

      void visitLValueType(LValueType* type) {
        if (debugPrint) {
          out << "LValue(";
          if (TypeBase* elem = type->getType())
            visit(elem);
          else
            out << nullTypeStr;
          out << ")";
        }
        else {
          out << "@";
          if (TypeBase* elem = type->getType())
            visit(elem);
          else
            out << nullTypeStr;
        }
      }

      void visitErrorType(ErrorType*) {
        out << "<error_type>";
      }

      void visitCellType(CellType* type) {
        // For constrained types, print the adress
        out << "Cell." << (void*)type << "(";

        // Print the contained type if there's one
        if (TypeBase* elem = type->getSubstitution()) 
          visit(elem);
        else out << nullTypeStr;
        out << ")";
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
  // This is ugly but needed. TypePrinter won't alter
  // this instance anyway so it's meaningless.
  tp.visit(const_cast<TypeBase*>(this));
  return oss.str();
}

TypeKind TypeBase::getKind() const {
  return kind_;
}

const TypeBase* TypeBase::unwrapIfArray() const {
  if (const ArrayType* tmp = dyn_cast<ArrayType>(this))
    return tmp->getElementType();
  return nullptr;
}

TypeBase* TypeBase::unwrapIfArray() {
  if (ArrayType* tmp = dyn_cast<ArrayType>(this))
    return tmp->getElementType();
  return nullptr;
}

const TypeBase* TypeBase::unwrapIfLValue() const {
  if (const LValueType* tmp = dyn_cast<LValueType>(this))
    return tmp->getType();
  return nullptr;
}

TypeBase* TypeBase::unwrapIfLValue() {
  if (LValueType* tmp = dyn_cast<LValueType>(this))
    return tmp->getType();
  return nullptr;
}

const TypeBase* TypeBase::ignoreLValue() const {
  auto* ptr = unwrapIfLValue();
  return ptr ? ptr : this;
}

TypeBase* TypeBase::ignoreLValue() {
  auto* ptr = unwrapIfLValue();
  return ptr ? ptr : this;
}

void* TypeBase::operator new(size_t sz, ASTContext& ctxt, std::uint8_t align) {
  return ctxt.getAllocator().allocate(sz, align);
}

//-------------//
// BuiltinType //
//-------------//

BuiltinType::BuiltinType(TypeKind tc):
  TypeBase(tc) {

}

//---------------//
// PrimitiveType //
//---------------//

PrimitiveType::PrimitiveType(Kind kd)
  : builtinKind_(kd), BuiltinType(TypeKind::PrimitiveType) {

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

bool PrimitiveType::isString() const {
  return builtinKind_ == Kind::StringTy;
}

bool PrimitiveType::isChar() const {
  return builtinKind_ == Kind::CharTy;
}

bool PrimitiveType::isBool() const {
  return builtinKind_ == Kind::BoolTy;
}

bool PrimitiveType::isInt() const {
  return builtinKind_ == Kind::IntTy;
}

bool PrimitiveType::isFloat() const {
  return builtinKind_ == Kind::FloatTy;
}

bool PrimitiveType::isVoid() const {
  return builtinKind_ == Kind::VoidTy;
}

//-----------//
// ArrayType //
//-----------//

ArrayType::ArrayType(TypeBase* elemTy):
  elementTy_(elemTy), BuiltinType(TypeKind::ArrayType) {
  assert(elemTy && "The Array item type cannot be null!");
}

ArrayType* ArrayType::get(ASTContext& ctxt, TypeBase* ty) {
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

TypeBase* ArrayType::getElementType() {
  return elementTy_;
}

const TypeBase* ArrayType::getElementType() const {
  return elementTy_;
}

//------------//
// LValueType //
//------------//

LValueType::LValueType(TypeBase* type):
  TypeBase(TypeKind::LValueType), ty_(type) {
  assert(type && "cannot be null");
}

LValueType* LValueType::get(ASTContext& ctxt, TypeBase* ty) {
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

TypeBase* LValueType::getType() {
  return ty_;
}

const TypeBase* LValueType::getType() const {
  return ty_;
}

//-----------//
// ErrorType //
//-----------//

ErrorType::ErrorType():
  TypeBase(TypeKind::ErrorType) {

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

TypeBase* CellType::getSubstitution() {
  return type_;
}

const TypeBase* CellType::getSubstitution() const {
  return type_;
}

bool CellType::hasSubstitution() const {
  return (type_ != nullptr);
}

void CellType::setSubstitution(TypeBase* type) {
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
