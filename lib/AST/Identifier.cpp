//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : Identifier.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/AST/Identifier.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/Common/Builtins.hpp"
#include "llvm/ADT/Hashing.h"
#include <ostream>

using namespace fox;

Identifier::Identifier(const char* ptr) : ptr_(ptr) {
	assert(ptr && "Can't create an Identifier with a null string. "
    "Use the default constructor for that!");
}

Identifier::Identifier() : ptr_(nullptr) {}

string_view Identifier::getStr() const {
  return string_view(ptr_);
}

bool Identifier::operator==(const Identifier& other) const {
	return ptr_ == other.ptr_;
}

bool Identifier::operator!=(const Identifier& other) const {
	return ptr_ != other.ptr_;
}

bool Identifier::operator<(const Identifier& other) const {
	return ptr_ < other.ptr_;
}

bool Identifier::operator==(const string_view other) const {
	return getStr() == other;
}

bool Identifier::operator!=(const string_view other) const {
	return getStr() != other;
}

bool Identifier::operator<(const string_view other) const {
	return getStr() < other;
}

const char* Identifier::c_str() const {
	return ptr_;
}

bool Identifier::isNull() const {
	return (ptr_ == nullptr);
}

Identifier::operator bool() const {
	return !isNull();
}

std::ostream& fox::operator<<(std::ostream& os, Identifier id) {
  os << (id.isNull() ? "<null id>" : id.getStr());
  return os;
}

std::size_t std::hash<Identifier>::operator()(Identifier ident) const {
  return llvm::hash_value(ident.c_str());
}