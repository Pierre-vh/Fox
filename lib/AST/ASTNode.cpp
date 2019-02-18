//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : ASTNode.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/AST/ASTNode.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/Common/SourceLoc.hpp"
#include "Fox/AST/Expr.hpp"
#include "Fox/AST/Stmt.hpp"
#include "Fox/AST/Decl.hpp"

using namespace fox;


SourceRange ASTNode::getSourceRange() const {
  if (is<Expr*>())
    return get<Expr*>()->getSourceRange();
  if (is<Decl*>())
    return get<Decl*>()->getSourceRange();
  if (is<Stmt*>())
    return get<Stmt*>()->getSourceRange();
  fox_unreachable("Unsupported node");
}

SourceLoc ASTNode::getBegin() const {
  return getSourceRange().getBegin();
}

SourceLoc ASTNode::getEnd() const {
  return getSourceRange().getEnd();
}