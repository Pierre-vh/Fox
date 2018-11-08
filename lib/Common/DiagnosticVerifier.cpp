//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : DiagnosticVerifier.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Common/DiagnosticVerifier.hpp"
#include "Fox/Common/Diagnostic.hpp"

using namespace fox;

//----------------------------------------------------------------------------//
//  DiagnosticVerifier's file parsing implementation
//----------------------------------------------------------------------------//

namespace {
  class FileParser {
    
  };
} // anonymous namespace

//----------------------------------------------------------------------------//
//  DiagnosticVerifier's methods implementation
//----------------------------------------------------------------------------//

DiagnosticVerifier::DiagnosticVerifier(SourceManager& srcMgr):
  srcMgr_(srcMgr) {
  
}

bool DiagnosticVerifier::parseFile(FileID) {
  return false;
}

void DiagnosticVerifier::consume(Diagnostic& diag) {
  // Check if there is an entry for this string in our map
  auto range = expectedDiags_.equal_range(diag.getStr());
  for (auto it = range.first; it != range.second; ++it) {
    // Found one, but check if the file & line match.
    std::pair<FileID, LineTy> pair = it->second;
    SourceLoc loc = diag.getRange().getBegin();

    // Check file match
    if (pair.first != loc.getFileID())
      continue;
    // Okay, file matches, now check the line.
    // This is the most expensive operation here so we do it last,
    // when we're sure that the string & file match.
    auto line = srcMgr_.getLineNumber(loc);
    if (line != pair.second)
      continue;

    // Diagnostic was expected, ignore it, remove the entry from the map
    // and return.
    expectedDiags_.erase(it);
    diag.ignore();
    return;
  }
}

void DiagnosticVerifier::addExpectedDiag(FileID file, LineTy line, 
  string_view str) {
  expectedDiags_.insert({str, {file, line}});
}
