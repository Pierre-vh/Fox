//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : StringManipulator.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// Implements a UTF-8 string manipulator based on the UTFCPP library.
//
// How it works :
//
// # = Current iterator iter_ position. It is always positioned at the end of the previous CP, and at the beginning of the current CP
// CP = One codepoint. It's one or more bytes in the std::string.
//    
//                            getCurrentChar()
//                              |->
//                        peekPrevious()|  peekNext()
//                          |->   |    |->
//                 - - --------------------------------------------------------- - -
//  str_(input string)         |  CP  |  CP  |  CP  #  CP  |  CP  |  CP  |  CP  |
//                 - - --------------------------------------------------------- - -
//
//
//----------------------------------------------------------------------------//

#pragma once

#include "Fox/Common/Typedefs.hpp"
#include "Fox/Common/string_view.hpp"

namespace fox {
  // This class is designed to help
  // with manipulating UTF8 strings.
  class StringManipulator {
    public:
      // Default ctor
      StringManipulator() = default;
      StringManipulator(string_view str);

      /*
        STRING GETTERS/SETTERS
      */  

      string_view getStr() const;
      void setStr(string_view str);

      /*
        ITERATOR MANIPULATION
      */
      void reset();
      void advance(const std::size_t& ind = 1);
      void goBack(const std::size_t& ind = 1);

      /*
        GET THE CURRENT CHARACTER
      */

      // Get the current codepoint
      FoxChar getCurrentChar() const;

      // Get a codepoint at a precise location
      FoxChar getChar(std::size_t ind) const;
      
      /*
        PEEK
      */

      FoxChar peekFirst() const;
      FoxChar peekNext() const;
      FoxChar peekPrevious() const;
      FoxChar peekBack() const;

      /*
        UTILS & OTHERS
      */

      // Extract a substring
			string_view substring(std::size_t beg, std::size_t leng) const;

      // Return the number of codepoints in the string
      std::size_t getSizeInCodepoints() const;

      // Returns the number of bytes in the string
      std::size_t getSizeInBytes() const;

      // Checks if the stringmanipulator has reached the end of the string
      bool eof() const;

      // Returns the index of the current character in codepoints
      // DO NOT MIX THIS WITH std::string::operator[] AND STRING OPERATIONS!
      std::size_t getIndexInCodepoints() const;

      // This uses std::distance to calculate the index at which the current codepoint begins in BYTES
      // You can use this with std::string::operator[] to retrieve the first byte of the codepoint.
      std::size_t getIndexInBytes() const;

      /*
        STATIC METHODS
      */

      // Convert a FoxChar to a utf8 encoded string
      static std::string charToStr(FoxChar wc);

      // Removes the BOM from a str
      static void removeBOM(std::string& str);

      // Appends a FoxChar to a std::string.
      static void append(std::string& str, FoxChar ch);

    private:
      // The string currently stored
      string_view str_;

      // Iterators
      string_view::const_iterator iter_, end_, beg_;
  };
}

