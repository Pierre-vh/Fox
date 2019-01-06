//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : PrintObjects.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the declaration for various ostream operators
// to teach googletest how to print some fox objects.
//----------------------------------------------------------------------------//

#pragma once

#include <ostream>

namespace fox {
  class SourceLoc;
  class SourceRange;
  struct CompleteLoc;
  class FileID;

  std::ostream& operator<<(std::ostream& os, FileID fid);
  std::ostream& operator<<(std::ostream& os, SourceLoc loc);
  std::ostream& operator<<(std::ostream& os, SourceRange range);
  std::ostream& operator<<(std::ostream& os, const CompleteLoc& cl);
}