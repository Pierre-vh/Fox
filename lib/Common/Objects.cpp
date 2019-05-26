//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : Objects.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Common/Objects.hpp"
#include "Fox/Common/UTF8.hpp"
#include "utfcpp/utf8.hpp"

using namespace fox;

//----------------------------------------------------------------------------//
// Object
//----------------------------------------------------------------------------//

ObjectKind Object::getKind() const {
  return kind_;
}

Object::Object(ObjectKind kind) : kind_(kind) {}

//----------------------------------------------------------------------------//
// StringObject
//----------------------------------------------------------------------------//

StringObject::StringObject() : StringObject(string_view()) {}

StringObject::StringObject(string_view value) 
  : Object(ObjectKind::StringObject), str_(value.to_string()) {}

const std::string& StringObject::str() const {
  return str_;
}

std::size_t StringObject::length() const {
  return utf8::distance(str_.begin(), str_.end());
}

std::size_t StringObject::numBytes() const {
  return str_.size();
}

FoxChar StringObject::getChar(std::size_t n) const {
  auto it = str_.begin();
  auto end = str_.end();
  utf8::advance(it, n, end);
  return utf8::peek_next(it, end);
}

//----------------------------------------------------------------------------//
// ArrayObject
//----------------------------------------------------------------------------//


fox::ArrayObject::ArrayObject(bool containsReferences, std::size_t minCapacity) 
  : Object(ObjectKind::ArrayObject), containsReferences_(containsReferences) {
  if(minCapacity != 0)
    data_.reserve(minCapacity);
}

std::size_t ArrayObject::size() const {
  return data_.size();
}

bool ArrayObject::containsReferences() const {
  return containsReferences_;
}

ArrayObject::ArrayT& ArrayObject::data() {
  return data_;
}

const ArrayObject::ArrayT& ArrayObject::data() const {
  return data_;
}
