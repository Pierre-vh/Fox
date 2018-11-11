//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : TestUtils.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file defines the Test utilities
//----------------------------------------------------------------------------//

#pragma once

#include <string>
#include <vector>
#include "PrintObjects.hpp"

namespace fox {
  namespace test {
    // Reads a file line by line and puts every line into a vector. 
    // Returns true in case of success, false if the file could not be opened.
    bool readFileToVec(const std::string& filepath, std::vector<std::string>& outvec);
    bool readFileToString(const std::string& filepath, std::string& outstr);
    std::string getPath(const std::string& relpath);
    std::string indent(const unsigned char& size);
  }
}
