//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : ASTNode.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/AST/ASTNode.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/Common/Source.hpp"
#include "Fox/Common/LLVM.hpp"
#include "Fox/AST/Expr.hpp"
#include "Fox/AST/Stmt.hpp"
#include "Fox/AST/Decl.hpp"

using namespace fox;

SourceRange ASTNode::getRange() const {
  if (is<Expr>())
    return get<Expr>()->getRange();
  if (is<Decl>())
    return get<Decl>()->getRange();
  if (is<Stmt>())
    return get<Stmt>()->getRange();
  fox_unreachable("Unsupported node");
}

SourceLoc ASTNode::getBegLoc() const {
  return getRange().getBegin();
}

SourceLoc ASTNode::getEndLoc() const {
  return getRange().getEnd();
}

bool ASTNode::isNullStmt() const {
  if (auto* ptr = getIf<Stmt>())
    return isa<NullStmt>(ptr);
  return false;
}