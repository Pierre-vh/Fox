//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : ASTNode.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file declares the ASTNode class.
//----------------------------------------------------------------------------//

#pragma once

#include "ASTAligns.hpp"
#include "llvm/ADT/PointerUnion.h"

namespace fox {
  class SourceRange;
  class SourceLoc;
  /// The ASTNode class is an intrusive pointer union that can store
  /// a Decl, Expr or Stmt pointer. This is used in places of the AST
  /// where we want to allow any kind of node.
  class ASTNode : public llvm::PointerUnion3<Expr*, Stmt*, Decl*> {
    public:
      using PointerUnion3::PointerUnion3;

      /// \returns the SourceRange of the Decl/Expr/Stmt
      SourceRange getSourceRange() const;
      /// \returns the begin SourceLoc of the Decl/Expr/Stmt
      SourceLoc getBeginLoc() const;
      /// \returns the end SourceLoc of the Decl/Expr/Stmt
      SourceLoc getEndLoc() const;
  };
}