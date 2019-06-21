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

Diagnostic VM::diagnose(DiagID diag) {
  assert(curFn_ && "Can't diagnose without a function being called");
  // Get the current instruction's index
  const Instruction* instrsBegin = curFn_->instrs_begin();
  std::size_t instrIdx = std::distance(instrsBegin, pc_);

  // Fetch the SourceRange (should always have one!)
  DebugInfo* dbg = curFn_->getDebugInfo();
  assert(dbg && "no debug info?");
  auto result = dbg->getSourceRange(instrIdx);
  assert(result.hasValue() && "no debug info for this instruction");

  // Stop execution & emit the error.
  actOnRuntimeError();
  return diagEngine.report(diag, result.getValue());
 
}