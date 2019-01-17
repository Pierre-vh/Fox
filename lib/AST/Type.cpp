//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
// See LICENSE.txt for license info.            
// File : Type.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/AST/Type.hpp"
#include "Fox/AST/Types.hpp"

using namespace fox;

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

//----------------------------------------------------------------------------//
// TypeLoc
//----------------------------------------------------------------------------//

TypeLoc::TypeLoc(Type ty, SourceRange range) :
  ty_(ty), range_(range) {}

SourceRange TypeLoc::getRange() const {
  return range_;
}

SourceLoc TypeLoc::getBegin() const {
  return range_.getBegin();
}

SourceLoc TypeLoc::getEnd() const {
  return range_.getEnd();
}

bool TypeLoc::isValid() const {
  return ty_ && range_;
}

TypeLoc::operator bool() const {
  return isValid();
}

Type TypeLoc::withoutLoc() {
  return ty_;
}

const Type TypeLoc::withoutLoc() const {
  return ty_;
}

//----------------------------------------------------------------------------//
// Other
//----------------------------------------------------------------------------//

std::ostream& fox::operator<<(std::ostream& os, Type ty) {
  assert(ty && "can't print a null type");
  os << ty->toString();
  return os;
}
