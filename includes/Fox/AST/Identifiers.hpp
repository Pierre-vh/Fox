///------------------------------------------------------////
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : Identifiers.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the IdentifierTable and Identifier classes.
//
// Note: This class, and the whole idiom of storing the Identifier* instead of the raw string
// is more or less a premature optimization. (I got the idea from CLang, found it was pretty nice and implemented it
// without thinking if it was really needed.) I did not realize that at the time of doing it,
// but now it's done so It's going to stay that way ! Currently the Identifier 
// doesn't store anything other than an iterator to the string, but in the future it might 
// contain much more as the language grows.
//
//----------------------------------------------------------------------------//

#pragma once 

#include <map>
#include <string>

namespace fox {
  class Identifier;

  // Wrapper around a const_iterator of a map entry, used to safely access the .first element (the string)
  class StringPtrInMap {
    public:
      typedef std::map<std::string, Identifier>::const_iterator ItTy;

      StringPtrInMap(ItTy iter);

      const std::string& get() const;

      ItTy it_;
  };

  // A lexed identifier.
  // Currently, they are immutable once created and only hold the string,
  // but in the future this might contain some more information.
  class Identifier {
    public:
      Identifier(const StringPtrInMap::ItTy& iter);

      Identifier(Identifier&&) = default;
      Identifier(Identifier&) = delete;
      Identifier& operator=(Identifier&) = delete;

      // Returns the string naming this identifier
      const std::string& getStr() const;

      // Comparison operators for use with STL containers.
      bool operator<(const Identifier& id) const;
      bool operator<(const std::string& idstr) const;

      // Other comparison operators
      bool operator==(const Identifier& id) const;
      bool operator==(const std::string& str) const;

      bool operator!=(const Identifier& id) const;
      bool operator!=(const std::string& str) const;
  
    private:
      friend class IdentifierTable;

      StringPtrInMap mapIter_;
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
      Identifier* getUniqueIdentifierInfo(const std::string& id);
      Identifier* getInvalidID();

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