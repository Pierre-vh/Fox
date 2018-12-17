//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : Type.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/AST/Type.hpp"
#include "Fox/AST/Types.hpp"

using namespace fox;

// Type

Type::Type(TypeBase* ty) :
  ty_(ty) {}

TypeBase* Type::getPtr() const {
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

// TypeLoc

TypeLoc::TypeLoc(TypeBase* ty, SourceRange range):
  Type(ty), range_(range) {

}

TypeLoc::TypeLoc(Type ty, SourceRange range) :
  Type(ty), range_(range) {

}

SourceRange TypeLoc::getRange() const {
  return range_;
}

Type TypeLoc::withoutLoc() {
  return Type(getPtr());
}

const Type TypeLoc::withoutLoc() const {
  // Remove the const attribute. It's meaningless since
  // we're returning a const Type and a const Type will always
  // return const pointers. This allows us to create the type.
  return Type(const_cast<TypeBase*>(getPtr()));
}

std::ostream& fox::operator<<(std::ostream& os, Type ty) {
  assert(ty && "can't print a null type");
  os << ty->toString();
  return os;
}
