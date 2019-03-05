//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : BCGen.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/BCGen/BCGen.hpp"
#include "Fox/AST/ASTContext.hpp"

using namespace fox;

BCGen::BCGen(ASTContext& ctxt) : ctxt_(ctxt) {}

DiagnosticEngine& BCGen::getDiagnosticEngine() const {
  return ctxt_.diagEngine;
}

ASTContext& BCGen::getASTContext() const {
  return ctxt_;
}
