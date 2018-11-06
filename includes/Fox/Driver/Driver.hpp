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
// (By try to compile it, I mean, with what's available. At the time of writing theses
// lines, the driver attempts to parse a single function definition and It'll output
// the AST and other Diag informations.)
//----------------------------------------------------------------------------//

#include <string>
#include <ostream>

namespace fox
{
  class Driver
  {
    public:  
      Driver() = default;
      // This function will lex a file and call parseFuncDecl, then output the ast.
      bool processFile(std::ostream& out, const std::string& filepath);
  };
}