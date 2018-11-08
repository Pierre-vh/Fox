//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : DiagnosticVerifier.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Common/DiagnosticVerifier.hpp"
#include "Fox/Common/Diagnostic.hpp"

using namespace fox;

DiagnosticVerifier::DiagnosticVerifier(SourceManager& srcMgr):
  srcMgr_(srcMgr) {
  
}

bool DiagnosticVerifier::parseFile(FileID) {
  return false;
}

bool DiagnosticVerifier::verify(Diagnostic& diag) {
  // Check if there is an entry for this string in our map
  auto it = expectedDiags_.find(diag.getStr());
  if (it != expectedDiags_.end()) {
    // Found one, but check if the file & line match.
    std::pair<FileID, LineTy> pair = it->second;
    SourceLoc loc = diag.getRange().getBegin();

    // Check file match
    if (pair.first != loc.getFileID())
      return true;
    // Okay, file matches, now check the line.
    // This is the most expensive operation here so we do it last,
    // when we're sure that the string & file match.
    auto line = srcMgr_.getLineNumber(loc);
    if (line != pair.second)
      return true;

    // Diagnostic was expected, return false (don't emit it)
    // and remove the entry from the map.
    expectedDiags_.erase(it);
    return false;
  }
  return true;
}

void DiagnosticVerifier::addExpectedDiag(FileID file, LineTy line, 
  string_view str) {
  expectedDiags_.insert({str, {file, line}});
}
