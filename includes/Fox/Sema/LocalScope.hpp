//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : LocalScope.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the LocalScope class.
//----------------------------------------------------------------------------//

#pragma once

#include "Fox/AST/Identifier.hpp"

#include <map>

namespace fox {
  class NamedDecl;

  // This class represents a single local scope.
  //
  // Theses are created by Sema when it enters a FuncDecl and Stmts that
  // open a new scope, such as ConditionStmt, WhileStmt, etc.
  //
  // You can only insert local declarations inside a LocalScope, and if
  // you insert a declaration that shares the same name as one already
  // existing in this instance, it'll be overwritten.
  class LocalScope {
    public:
      using MapTy = std::map<Identifier, NamedDecl*>;
      
      LocalScope(LocalScope* parent = nullptr);

      // Adds a declaration in this LocalScope. 
      // Returns true if the insertion occured, false
      // if a NamedDecl* with the same Identifier prevented the insertion.
      //
      // Note that "decl" must have a valid Identifier()
      bool add(NamedDecl* decl);

      // Adds a declaration in this LocalScope. 
      // Returns false if the declaration replaced a previous one, true
      // if the insertion occured without overwriting any previous decl.
      // 
      // Note that "decl" must have a valid Identifier()
      bool forceAdd(NamedDecl* decl);

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