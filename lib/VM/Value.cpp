//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : Value.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/VM/Value.hpp"

using namespace fox;

bool Value::isNull() const {
  return pointers_.isNull();
}

bool Value::isInt() const {
  return pointers_.is<BoxedPrimtive<FoxInt>*>();
}

bool Value::isDouble() const {
  return pointers_.is<BoxedPrimtive<FoxDouble>*>();
}

bool Value::isChar() const {
  return pointers_.is<BoxedPrimtive<FoxChar>*>();
}
