//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : Identifier.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the Identifier class.
//----------------------------------------------------------------------------//

#pragma once 

#include "Fox/Common/string_view.hpp"
#include <iosfwd>

namespace fox {
  // Represents a unique'd lexed identifier
  // They are immutable once created and only contain a
	// null-terminated const char *
  //
  // This essentially exists as an optimization. This object is the
	// size of a pointer so passing it by value is okay.
  class Identifier {
    public:
			// Create a null Identifier
			Identifier();

      string_view getStr() const;
			const char* c_str() const;

			bool isNull() const;
			// Returns !isNull
			operator bool() const;
  
			// Compare with another Identifier
			bool operator==(const Identifier& other) const;
			bool operator!=(const Identifier& other) const;
			bool operator<(const Identifier& other) const;

			// Compare with a string_view
			bool operator==(const string_view other) const;
			bool operator!=(const string_view other) const;
			bool operator<(const string_view other) const;

    private:
			// Only the context can create a non null Identifier object.
			friend class ASTContext;

			// Creates a new Identifier object. The pointer can't be null,
      // and must point to a null-terminated C-Style string.
      Identifier(const char* ptr);

      // The stored pointer, either null, or pointing at
      // a null-terminated C-Style string.
      const char* ptr_ = nullptr;
  };

  std::ostream& operator<<(std::ostream& os, Identifier id);
}

namespace std {
  template<>
  struct hash<::fox::Identifier> {
    std::size_t operator()(::fox::Identifier ident) const;
  };
}