//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : Objects.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Common/Objects.hpp"

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

std::string& StringObject::str() {
  return str_;
}

const std::string& StringObject::str() const {
  return str_;
}
