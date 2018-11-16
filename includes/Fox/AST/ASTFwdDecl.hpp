//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : ASTFwdDecl.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file does a forward declaration of every node in the AST
// hierarchy. This includes:
//    Any abstract or concrete:
//      Expr
//      Stmt
//      Decl
//      Type
//    Constraint
//    ASTNode
//----------------------------------------------------------------------------//

#pragma once

namespace fox {
  class ASTNode;

  class Expr;
  #define EXPR(ID, PARENT) class ID;
  #define ABSTRACT_EXPR(ID, PARENT) class ID;
  #include "ExprNodes.def"

  class Decl;
  #define DECL(ID, PARENT) class ID;
  #define ABSTRACT_DECL(ID, PARENT) class ID;
  #include "DeclNodes.def"

  class Stmt;
  #define STMT(ID, PARENT) class ID;
  #define ABSTRACT_STMT(ID, PARENT) class ID;
  #include "StmtNodes.def"

  class TypeBase;
  #define TYPE(ID, PARENT) class ID;
  #define ABSTRACT_TYPE(ID, PARENT) class ID;
  #include "TypeNodes.def"

  class Type;
  class TypeLoc;
}