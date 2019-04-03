//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : UTF8.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Common/UTF8.hpp"
#include "utfcpp/utf8.hpp"

using namespace fox;

void fox::appendFoxChar(FoxChar ch, std::string& dest) {
  utf8::append(ch, std::back_inserter(dest));
}