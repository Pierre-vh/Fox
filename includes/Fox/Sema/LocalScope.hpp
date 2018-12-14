//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : LocalScope.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the LocalScope class.
//----------------------------------------------------------------------------//

#pragma once

#include "Fox/AST/DeclContext.hpp"
#include "Fox/AST/Identifier.hpp"

namespace fox {
  class NamedDecl;

  // This class represents a single local scope.
  //
  // Theses are created by Sema when it enters a FuncDecl and Stmts that
  // open a new scope, such as ConditionStmt, WhileStmt, etc.
  //
  // You can only insert local declarations inside a LocalScope.
  //
  // Note that this class is pretty trivial. It won't check if a decl
  // was inserted twice or anything like that, and doesn't offer any lookup
  // interface on it's own.
  class LocalScope {
    public:
      // Use the same map as DeclContext
      using MapTy = DeclContext::MapTy;
      
      LocalScope(LocalScope* parent = nullptr);

      // Adds a declaration in this LocalScope.
      // Note that "decl" must have a valid Identifier()
      void add(NamedDecl* decl);

      // Returns the map of (Identifier -> NamedDecl*) used internally to
      // store declarations in this scope.
      MapTy& getDeclsMap();

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