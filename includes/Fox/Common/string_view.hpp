//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
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
