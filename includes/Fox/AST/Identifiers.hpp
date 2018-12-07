//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : Identifiers.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the IdentifierTable and Identifier classes.
//----------------------------------------------------------------------------//

#pragma once 

#include <map>
#include "Fox/Common/string_view.hpp"

namespace fox {
  // Represents a unique'd lexed identifier
  // Currently, they are immutable once created and only hold the string,
  // but in the future this might contain more than that.
  //
  // This essentially exists as an optimization.
  class Identifier {
    public:
      Identifier(Identifier&&) = default;

      // Returns the string naming this identifier
      string_view getStr() const;

      // Comparison operators for use with STL containers.
      bool operator<(const Identifier& id) const;
      bool operator<(const std::string& idstr) const;

      // Other comparison operators
      bool operator==(const Identifier& id) const;
      bool operator==(const std::string& str) const;

      bool operator!=(const Identifier& id) const;
      bool operator!=(const std::string& str) const;
  
    private:
      Identifier(string_view str = string_view());
      Identifier(Identifier&) = delete;
      Identifier& operator=(Identifier&) = delete;

      friend class IdentifierTable;

      string_view str_;
  };

  // A class that maps strings to Identifier.
  // This contains every (user-defined) identifier currently in use, and is populated by the 
  // Lexer.
  class IdentifierTable {
    private:
      using IDTableType = std::map<std::string, Identifier>;
      using IDTableIteratorType = IDTableType::iterator;
      using IDTableConstIteratorType = IDTableType::const_iterator;

    public:
      IdentifierTable() = default;

      // Returns the identifierinfo of the string "id" if it exists. 
      // If it does not exists, it creates a new entry into the table and returns it.
      Identifier* getIdentifier(const std::string& id);

      // Returns true if the identifier exists in the map, false otherwise.
      bool exists(const std::string &id) const;

      // Iterators
      IDTableConstIteratorType begin() const;
      IDTableIteratorType begin();

      IDTableConstIteratorType end() const;
      IDTableIteratorType end();

    private:
      Identifier* invalidID_ = nullptr;

      // Deleted methods
      IdentifierTable(const IdentifierTable&) = delete;
      IdentifierTable& operator=(const IdentifierTable&) = delete;

      // Member variables
      IDTableType table_;
  };
}