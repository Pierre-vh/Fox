//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : QuotedString.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Common/QuotedString.hpp"

using namespace fox;

namespace {
  void printSingleChar(char ch, std::ostream& out, char delim) {
    if(ch == (unsigned char)delim)
      out << '\\' << ch;
    else {
      switch (ch) {
        case 0:
          out << "\\0";
          break;
        case '\n':
          out << "\\n";
          break;
        case '\r':
          out << "\\r";
          break;
        case '\t':
          out << "\\t";
          break;
        case '\\':
          out << "\\\\";
          break;        
        default:
          out << ch;
          break;
      }
    }
  }
}

void fox::printQuotedString(string_view str, std::ostream& out, char delim) {
  if(delim) out << delim;
  for (char ch : str) 
    printSingleChar(ch, out, delim);
  if(delim) out << delim;
}