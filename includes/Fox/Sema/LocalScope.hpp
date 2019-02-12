//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : LocalScope.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the LocalScope class.
//----------------------------------------------------------------------------//

#pragma once

#include "Fox/AST/Identifier.hpp"
#include "Fox/AST/ASTAligns.hpp"
#include "llvm/ADT/PointerUnion.h"
#include <map>

namespace fox {
  class NamedDecl;
  class FuncDecl;
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
      using Map = std::map<Identifier, NamedDecl*>;
      using Parent = llvm::PointerUnion<LocalScope*, FuncDecl*>;

      LocalScope(Parent parent = (LocalScope*)nullptr);

      // Inserts a declaration in this LocalScope. 
      // Returns false if the declaration replaced a previous one, true
      // if the insertion occured without overwriting any previous decl.
      // 
      // Note that "decl" must have a valid Identifier()
      bool insert(NamedDecl* decl);

      // If the parent is a LocalScope*, return it. Else, return nullptr.
      LocalScope* getParentIfLocalScope() const;

      // Returns the root FuncDecl of this Scope tree
      FuncDecl* getFuncDecl() const;

      // Returns the map of (Identifier -> NamedDecl*) used internally to
      // store declarations in this scope.
      Map& getDeclsMap();

      bool hasParent() const;
      void setParent(Parent scope);
      Parent getParent() const;

    private:
      // The parent scope
      llvm::PointerUnion<LocalScope*, FuncDecl*> parent_;
      // The decls present in this scope
      Map decls_;
  };
}