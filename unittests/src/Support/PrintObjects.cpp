//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : PrintObjects.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "PrintObjects.hpp"
#include "Fox/Common/Source.hpp"

using namespace fox;

std::ostream& fox::operator<<(std::ostream& os, FileID fid) {
  os << "FileID(" << fid.getRaw() << ")";
  return os;
}

std::ostream& fox::operator<<(std::ostream& os, SourceLoc loc) {
  os << "SourceLoc(" << loc.getFile() << ", " << loc.getIndex() << ")";
  return os;
}

std::ostream& fox::operator<<(std::ostream& os, SourceRange range) {
  os << "SourceRange(" << range.getBegin() << ", " << range.getEnd() << ")";
  return os;
}

std::ostream& fox::operator<<(std::ostream& os, const CompleteLoc& loc) {
  os << "CompleteLoc(" << loc.fileName << ", " << loc.line << ", " << loc.column << ")";
  return os;
}
