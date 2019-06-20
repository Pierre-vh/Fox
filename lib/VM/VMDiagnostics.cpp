//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : VMDiagnostics.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// Implements diagnostics-related functionality for the VM
//----------------------------------------------------------------------------//

#include "Fox/VM/VM.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"

using namespace fox;

Diagnostic VM::diagnose(DiagID diag) const {
  // TO-DO: Use DebugInfo for the current instruction to emit a more precise
  // diagnostic

  return diagEngine.report(diag, SourceRange());
  
  // FIXME: Sometimes we might want to emit notes after the diag is emitted.
  // To do that we'd need to have a "callback" function that is called once
  // the diagnostic has been emitted. This needs modification to the 
  // Diagnostic object.

  // TO-DO: Stop execution
}