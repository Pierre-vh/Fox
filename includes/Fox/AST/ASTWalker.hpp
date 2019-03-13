//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : ASTWalker.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the ASTWalker and TypeWalker classes, which are used to
// "walk" the ast in a pre/post order fashion.
//----------------------------------------------------------------------------//

#pragma once

#include <tuple>
#include "Fox/AST/ASTFwdDecl.hpp"

namespace fox {
  // The ASTWalker which traverses an AST, visiting every Expr/Decl and Stmt
  // nodes.
  class ASTWalker {
    public:
      // Walks an ASTNode. 
      // The return value of the walk will be discarded.
      void walk(ASTNode node);

      // Walks an Expr
      // Returns it's argument, or another Expr that should
      // take it's place, or nullptr if the walk was aborted.
      Expr* walk(Expr* expr);

      // Walks a Decl
      // Returns true on success, false if the walk was aborted.
      bool walk(Decl* decl);
      
      // Walks a Stmt
      // Returns it's argument, or another Stmt that should
      // take it's place, or nullptr if the walk was aborted.
      Stmt* walk(Stmt* stmt);

      // Called when first visiting an expression before visiting it's
      // children. 
      // The first element of the return pair is the node that should
      // take this node's place, if it's nullptr, the traversal is terminated.
      // The second element is a boolean indicating if we should visit this
      // node's children.
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
      // The second element is a boolean indicating if we should visit this 
      // node's children.
      virtual std::pair<Stmt*, bool> handleStmtPre(Stmt* stmt);


      // Called after visiting a statement's children.
      // If the return value is null, the traversal is terminated, otherwise
      // the walked node is replaced by the returned node.
      // The default implementation returns it's argument.
      virtual Stmt* handleStmtPost(Stmt* stmt);

      // Called when first visiting a declaration before visiting it's
      // children. 
      // Return true if we should walk into the children of this decl,
      // false if the walk should be aborted.
      // The default implementation returns true.
      virtual bool handleDeclPre(Decl* decl);


      // Called after visiting a declaration's children.
      // Return true if we should walk into the children of this decl,
      // false if the walk should be aborted.
      virtual bool handleDeclPost(Decl* decl);
  };
}
