//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
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
  class ASTNode : public llvm::PointerUnion3<Expr*, Stmt*, Decl*> {
    public:
      using PointerUnion3::PointerUnion3;

      SourceRange getSourceRange() const;
      SourceLoc getBegin() const;
      SourceLoc getEnd() const;
  };
}