//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : Sema.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  This file implements Sema methods that aren't tied to Expression,
//  Statements, Declarations or Types.
//----------------------------------------------------------------------------//

#include "Fox/Sema/Sema.hpp"

using namespace fox;

Sema::Sema(ASTContext& ctxt, DiagnosticEngine& diags) :
  ctxt_(ctxt), diags_(diags) {

}

DiagnosticEngine& Sema::getDiagnosticEngine() {
  return diags_;
}

ASTContext& Sema::getASTContext() {
  return ctxt_;
}
