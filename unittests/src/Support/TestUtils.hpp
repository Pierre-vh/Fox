//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : TestUtils.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file defines the Test utilities
//----------------------------------------------------------------------------//

#pragma once

#include <string>
#include <vector>

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
