//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : string_view.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file imports the currently used C++17-like string_view library. 
//----------------------------------------------------------------------------//

#include "nonstd/string_view.hpp"

namespace fox {
  // We want to use it unqualified
  using string_view = nonstd::string_view;
}
