//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : Identifiers.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the Identifier classes.
//----------------------------------------------------------------------------//

#pragma once 

#include "Fox/Common/string_view.hpp"

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
			// Creates a new Identifier object. 
			// /!\ : the pointer must not be null, and point to a null-terminated
			//			 c-style string!
      Identifier(const char* ptr);

      const char* ptr_ = nullptr;
  };
}