//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : Value.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/VM/Value.hpp"
#include "Fox/Common/Errors.hpp"

using namespace fox;

Value::Value(FoxInt value) {
  setInt(value);
}

Value::Value(FoxDouble value) {
  setDouble(value);
}

Value::Value(FoxChar value) {
  setChar(value);
}

Value::Value(const Value& value) {
  deepCopy(value);
}

Value::Value(Value&& value) : pointers_(value.pointers_) {
  value.pointers_ = nullptr;
}

Value& Value::operator=(const Value& value) {
  deepCopy(value);
  return *this;
}

Value::~Value() {
  reset();
}

void Value::setInt(FoxInt value) {
  // Reset if not an int
  if(!isInt()) reset();
  // Set or create the value
  if(isNull())
    pointers_ = new IntBox(value);
  else 
    pointers_.get<IntBox*>()->value = value;
}

void Value::setDouble(FoxDouble value) {
  // Reset if not a double
  if(!isDouble()) reset();
  // Set or create the value
  if(isNull())
    pointers_ = new DoubleBox(value);
  else 
    pointers_.get<DoubleBox*>()->value = value;
}

void Value::setChar(FoxChar value) {
  // Reset if not a char
  if(!isChar()) reset();
  // Set or create the value
  if(isNull())
    pointers_ = new CharBox(value);
  else 
    pointers_.get<CharBox*>()->value = value;
}

FoxInt Value::getInt() const {
  assert(isInt() && "Not an int!");
  return pointers_.get<IntBox*>()->value;
}

FoxDouble Value::getDouble() const {
  assert(isDouble() && "Not a double!");
  return pointers_.get<DoubleBox*>()->value;
}

FoxChar Value::getChar() const {
  assert(isChar() && "Not a char!");
  return pointers_.get<CharBox*>()->value;
}

bool Value::isNull() const {
  return pointers_.isNull();
}

bool Value::isInt() const {
  return !isNull() && pointers_.is<IntBox*>();
}

bool Value::isDouble() const {
  return !isNull() && pointers_.is<DoubleBox*>();
}

bool Value::isChar() const {
  return !isNull() && pointers_.is<CharBox*>();
}

void Value::reset() {
  if(isUniquelyOwned())
    deleteOwnedValue();
}

bool Value::isUniquelyOwned() const {
  // Until I have "objects" (array/strings),
  // everything is uniquely owned.
  return !isNull(); // Return true if not null
}

void Value::deleteOwnedValue() {
  assert(isUniquelyOwned() && "Value not eligible for deletion");
  if(isInt())
    delete pointers_.get<IntBox*>();
  else if(isDouble())
    delete pointers_.get<DoubleBox*>();
  else if(isChar())
    delete pointers_.get<CharBox*>();
  else 
    fox_unreachable("unknown uniquely owned value kind");
  pointers_;
}

void Value::deepCopy(const Value& other) {
  if(other.isNull())
    reset();
  else if (other.isUniquelyOwned()) {
    if(other.isInt())
      setInt(other.getInt());
    if(other.isDouble())
      setDouble(other.getDouble());
    if(other.isChar())
      setChar(other.getChar());
  }
  fox_unreachable("non-uniquely owned values not supported yet");
}