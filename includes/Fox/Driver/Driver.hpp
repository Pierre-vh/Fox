//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : Driver.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This class aims to provide a basic driving tool for the compiler
// 
// At first, this driver will just take a file path as input and try
// to compile it, emitting informations to a user-defined ostream.
//----------------------------------------------------------------------------//

#include <string>
#include <ostream>

namespace fox {
  class Driver {
    public:  
      Driver() = default;
      bool processFile(std::ostream& out, const std::string& filepath);
  };
}
