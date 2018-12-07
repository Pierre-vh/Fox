//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : Identifiers.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/AST/Identifiers.hpp"
#include "Fox/Common/Errors.hpp"

using namespace fox;

Identifier::Identifier(string_view str): str_(str) {

}

string_view Identifier::getStr() const {
  return str_;
}

Identifier* IdentifierTable::getIdentifier(const std::string& id) {
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

bool IdentifierTable::exists(const std::string& id) const {
  // Return false if there is no identifier in the table
  if (table_.size()) {
    auto it = table_.find(id);
    return (it != table_.end());
  }
  return false;
}