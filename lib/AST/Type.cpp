//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
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
// Other
//----------------------------------------------------------------------------//

std::ostream& fox::operator<<(std::ostream& os, Type ty) {
  assert(ty && "can't print a null type");
  os << ty->toString();
  return os;
}
