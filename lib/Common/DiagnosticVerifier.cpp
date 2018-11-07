//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : DiagnosticVerifier.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Common/DiagnosticVerifier.hpp"
#include "..\..\includes\Fox\Common\DiagnosticVerifier.hpp"

using namespace fox;



DiagnosticVerifier::DiagnosticVerifier(SourceManager& srcMgr,
  std::unique_ptr<DiagnosticConsumer> consumer):
  srcMgr_(srcMgr), consumer_(std::move(consumer)) {
  
}

void DiagnosticVerifier::consume(Diagnostic&) {

}

bool DiagnosticVerifier::parseFile(FileID) {
  return false;
}
