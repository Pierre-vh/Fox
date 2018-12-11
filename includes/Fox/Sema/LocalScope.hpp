//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : LocalScope.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the LocalScope class.
//----------------------------------------------------------------------------//

#pragma once

#include <map>
#include <vector>
#include "Fox/AST/Identifier.hpp"

namespace fox {
  class NamedDecl;

  // This class represents a single scope, and contains pointer a pointer to
  // the parent scope. This is pretty similar to a DeclContext, but simplified.
  //
  // Theses are created by Sema when it enters a local DeclContext.
  //
  // Note that this class is pretty trivial. It won't check if a decl
  // was inserted twice or anything like that, and doesn't offer any lookup
  // interface on it's own.
  class LocalScope {
    public:
      using MapTy = std::multimap<Identifier, NamedDecl*>;
      
      LocalScope(LocalScope* parent = nullptr);

      // Adds a declaration in this LocalScope.
      // Note that "decl" must have a valid Identifier()
      void add(NamedDecl* decl);

      // Returns the map of (Identifier -> NamedDecl*) used internally to
      // store declarations in this scope.
      MapTy& getMap();

      LocalScope* getParent() const;
      bool hasParent() const;
      void setParent(LocalScope* scope);

    private:
      // The parent scope
      LocalScope* parent_ = nullptr;
      // The decls present in this scope
      MapTy decls_;
  };
}