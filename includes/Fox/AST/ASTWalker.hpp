//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : ASTWalker.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file declares the ASTWalker class.
//----------------------------------------------------------------------------//

#pragma once

#include <tuple>
#include "Fox/AST/ASTFwdDecl.hpp"

namespace fox {
  /// The ASTWalker is a class that implements a pre/post-order traversal
  /// of the AST, visiting every Expr/Stmt/Decl.
  class ASTWalker {
    public:
      /// Walks an ASTNode. 
      /// NOTE: The return value of the walk will be discarded.
      /// \param node the node to visit
      void walk(ASTNode node);

      /// Walks an Expr
      /// \param expr the expr to visit
      /// \returns its parameter, another expr that should take its place or
      ///          nullptr if the walk was terminated early.
      Expr* walk(Expr* expr);

      /// Walks a Decl
      /// \param decl the decl to visit
      /// \returns true on success, false if the walk was aborted.
      bool walk(Decl* decl);
      
      /// Walks a Stmt
      /// \param stmt the stmt to visit
      /// \returns the stmt, another stmt that should take its place or
      ///          nullptr if the walk was aborted.
      Stmt* walk(Stmt* stmt);

      /// Called when first visiting an expression before visiting its
      /// children. 
      /// \param expr the expression we're visiting
      /// \returns a pair: the first element is the expr that should
      ///          take this expr's place, if it's nullptr, the traversal
      ///          is terminated. The second element is a boolean indicating
      ///          if we should visit this node's children.
      virtual std::pair<Expr*, bool> handleExprPre(Expr* expr);

      /// Called after visiting an expression's children
      /// \param expr the expr we're visiting
      /// \returns its parameter, another expr that should take its place or
      ///          nullptr to abort the walk.
      virtual Expr* handleExprPost(Expr* expr);

      /// Called when first visiting a statement before visiting its
      /// children. 
      /// \param stmt the statement we're visiting
      /// \returns a pair: the first element is the stmt that should
      ///          take this stmt's place, if it's nullptr, the traversal
      ///          is terminated. The second element is a boolean indicating
      ///          if we should visit this node's children.
      virtual std::pair<Stmt*, bool> handleStmtPre(Stmt* stmt);


      /// Called after visiting an statement's children
      /// \param stmt the stmt we're visiting
      /// \returns its parameter, another stmt that should take its place or
      ///          nullptr if the walk was aborted.
      virtual Stmt* handleStmtPost(Stmt* stmt);

      /// Called before visiting a declaration's children.
      /// \param decl the Decl that we're visiting
      /// \returns true if we should continue the traversal, false otherwise.
      virtual bool handleDeclPre(Decl* decl);

      /// Called after visiting a declaration's children.
      /// \param decl the Decl that we're visiting
      /// \returns true if we should continue the traversal, false otherwise.
      virtual bool handleDeclPost(Decl* decl);
  };
}
