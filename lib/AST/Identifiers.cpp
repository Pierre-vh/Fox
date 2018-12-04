//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : Identifiers.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/AST/Identifiers.hpp"
#include "Fox/Common/Errors.hpp"

// Normally this identifier shouldn't be possible
// in the language because <> are both illegal in an identifier
#define INVALID_ID_STR "<invalid>"

using namespace fox;

Identifier::Identifier(string_view str): str_(str) {

}

string_view Identifier::getStr() const {
  return str_;
}

bool Identifier::operator<(const Identifier& id) const {
  return getStr() < id.getStr();
}

bool Identifier::operator<(const std::string& idstr) const {
  return getStr() < idstr;
}

bool Identifier::operator==(const Identifier& id) const {
  return getStr() == id.getStr();
}

bool Identifier::operator==(const std::string& str) const {
  return getStr() == str;
}

bool Identifier::operator!=(const Identifier& id) const {
  return !(*this == id);
}

bool Identifier::operator!=(const std::string& str) const {
  return !(*this == str);
}

Identifier* IdentifierTable::getUniqueIdentifierInfo(const std::string& id) {
  auto it = table_.lower_bound(id);
  if (it != table_.end() && !(table_.key_comp()(id, it->first))) {
    // Identifier instance already exists in table_, return ->second after some checks.
    
    // Sanity check
    assert((it->first == it->second.getStr())
      && "Strings don't match!");

    return &(it->second);
  }
  else {
    // Key does not exists, insert.
    auto newIt = table_.insert(it, std::make_pair(id, Identifier()));
    // /!\ Important : Set the string view to watch the string
    // stored inside the map
    newIt->second.str_ = newIt->first;
    return &(newIt->second);
  }
}

Identifier* IdentifierTable::getInvalidID() {
  if (!invalidID_)
    invalidID_ = getUniqueIdentifierInfo(INVALID_ID_STR);
  assert(invalidID_ && "invalidID_ cannot be null!");
  return invalidID_;
}

bool IdentifierTable::exists(const std::string& id) const {
  // Return false if there is no identifier in the table
  if (table_.size()) {
    auto it = table_.find(id);
    return (it != table_.end());
  }
  return false;
}

IdentifierTable::IDTableConstIteratorType IdentifierTable::begin() const {
  return table_.begin();
}

IdentifierTable::IDTableIteratorType IdentifierTable::begin() {
  return table_.begin();
}

IdentifierTable::IDTableConstIteratorType IdentifierTable::end() const {
  return table_.end();
}

IdentifierTable::IDTableIteratorType IdentifierTable::end() {
  return table_.end();
}