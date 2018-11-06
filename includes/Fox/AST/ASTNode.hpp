//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : ASTNode.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// The ASTNode is a class that acts like a variant of
// Expr/Stmt/Decl pointers.
// This is used in places where we allow any node kind: Expr,
// Decl or Stmt.
//----------------------------------------------------------------------------//

#pragma once

#include "Fox/Common/PtrVariant.hpp"

namespace fox {
  class Expr;
  class Stmt;
  class Decl;
  class SourceRange;
  class SourceLoc;
  class ASTNode : public PtrVariant<Expr, Stmt, Decl> {
    public:
      using PtrVariant::PtrVariant;

      SourceRange getRange() const;
      SourceLoc getBegLoc() const;
      SourceLoc getEndLoc() const;

      // Common helpers
      bool isNullStmt() const;
  };
}