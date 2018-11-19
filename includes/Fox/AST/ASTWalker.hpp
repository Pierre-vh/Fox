//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : ASTWalker.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the ASTWalker class, which is used to
// "walk" the ast in a pre/post order fashion.
//----------------------------------------------------------------------------//

#pragma once

#include <tuple>
#include "Fox/AST/ASTFwdDecl.hpp"

namespace fox {
  class ASTWalker {
    public:
      ASTNode walk(ASTNode node);
      Expr* walk(Expr* expr);
      Decl* walk(Decl* decl);
      Stmt* walk(Stmt* stmt);
      void walk(TypeBase* type);

      // Called when first visiting an expression before visiting it's
      // children. 
      // The first element of the return pair is the node that should
      // take this node's place, if it's nullptr, the traversal is terminated.
      // The second element is a boolean indicating if we should visit this node's
      // children.
      virtual std::pair<Expr*, bool> handleExprPre(Expr* expr);

      // Called after visiting an expression's children.
      // If the return value is null, the traversal is terminated, otherwise
      // the walked node is replaced by the returned node.
      // The default implementation returns it's argument.
      virtual Expr* handleExprPost(Expr* expr);

      // Called when first visiting a statement before visiting it's
      // children. 
      // The first element of the return pair is the node that should
      // take this node's place, if it's nullptr, the traversal is terminated.
      // The second element is a boolean indicating if we should visit this node's
      // children.
      virtual std::pair<Stmt*, bool> handleStmtPre(Stmt* stmt);


      // Called after visiting a statement's children.
      // If the return value is null, the traversal is terminated, otherwise
      // the walked node is replaced by the returned node.
      // The default implementation returns it's argument.
      virtual Stmt* handleStmtPost(Stmt* stmt);

      // Called when first visiting a declaration before visiting it's
      // children. 
      // The first element of the return pair is the node that should
      // take this node's place, if it's nullptr, the traversal is terminated.
      // The second element is a boolean indicating if we should visit this node's
      // children.
      virtual std::pair<Decl*, bool> handleDeclPre(Decl* decl);


      // Called after visiting a declaration's children.
      // If the return value is null, the traversal is terminated, otherwise
      // the walked node is replaced by the returned node.
      // The default implementation returns it's argument.
      virtual Decl* handleDeclPost(Decl* decl);

      // Called when first visiting a type before visiting it's
      // children. 
      // If the return value is false, the traversal is terminated.
      virtual bool handleTypePre(TypeBase* type);

      // Called after visiting a type's children.
      // If the return value is false, the traversal is terminated.
      virtual bool handleTypePost(TypeBase* type);
  };
}