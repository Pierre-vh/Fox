//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : Scope.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file implements the Scope class, which is used to perform
// local variable binding.
//
// TODO: Change VarDecl to NamedDecl for the Scope
//----------------------------------------------------------------------------//

#pragma once

#include <map>
#include <vector>
#include "Fox/AST/Identifier.hpp"

namespace fox {
  class VarDecl;

  // This class represents a single scope, and contains pointer a pointer to
  // the parent scope. This is pretty similar to a DeclContext, but simplified.
  class Scope {
    public:
      using LookupResultTy = std::vector<VarDecl*>;
      
      // Adds a declaration so it's visible in this current scope
      // If a declaration with this name already exists in this scope, or it's
      // parent, the decl won't be added and we'll return the a pointer to 
      // the first (in lexical order) occurence of a decl with the same name. 
      VarDecl* add(VarDecl* decl);

      // Performs a lookup in this scope, populating the "result&"
      // vector with every found decl.
      //
      // The result vector is populated in a reverse order. The first result
      // is the latest occurence, the last result is the earliest one.
      void lookup(Identifier id, LookupResultTy& results);

      Scope* getParent() const;
      bool hasParent() const;
      void setParent(Scope* scope);

    private:
      // Searches only in this scope instance's map.
      void lookupImpl(Identifier id, LookupResultTy& results);

      // The parent scope
      Scope* parent_ = nullptr;
      // The decls present in this scope
      std::map<Identifier, VarDecl*> decls_;
  };
}