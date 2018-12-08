//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : ASTNode.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// The ASTNode is a class that acts like a variant of
// Expr/Stmt/Decl pointers.
// This is used in places where we want to allow any node kind:
// Expr, Decl or Stmt.
//----------------------------------------------------------------------------//

#pragma once

#include "ASTAligns.hpp"
#include "llvm/ADT/PointerUnion.h"

namespace fox {
  class SourceRange;
  class SourceLoc;
  class ASTNode {
    llvm::PointerUnion3<Expr*, Stmt*, Decl*> ptrs_;
    public:
      ASTNode();
      ASTNode(Expr* expr);
      ASTNode(Decl* decl);
      ASTNode(Stmt* stmt);

      SourceRange getRange() const;
      SourceLoc getBegLoc() const;
      SourceLoc getEndLoc() const;
      
      bool isNull() const;
      explicit operator bool() const;

      template<typename Ty>
      bool is() const {
        return ptrs_.is<Ty*>();
      }

      template<typename Ty>
      const Ty* getIf() const {
        return ptrs_.dyn_cast<Ty*>();
      }

      template<typename Ty>
      Ty* getIf() {
        return ptrs_.dyn_cast<Ty*>();
      }

      template<typename Ty>
      const Ty* get() const {
        return ptrs_.get<Ty*>();
      }

      template<typename Ty>
      Ty* get() {
        return ptrs_.get<Ty*>();
      }
  };
}