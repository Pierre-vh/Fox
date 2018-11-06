//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : FTT.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the main interface for the Fox Testing Tool.
//----------------------------------------------------------------------------//

#pragma once

#include <iostream>
#include <string>

namespace fox
{
namespace ftt
{
  /*
    Base class for every test class
  */
  class Test
  {
    public:
      // Return true if we're done testing, false otherwise.
      bool isDone() const;

      // Returns true if the test passed, false otherwise.
      // If isDone returns false, this will return false too.
      bool hasPassed() const;

    protected:
      // Constructs a FileTest object 
      // Note that no test result is going to be printed to the stream,
      // only diagnostics/errors.
      Test(std::ostream& os = std::cout);

      // Returns the out stream used to print errors and
      // diagnostics.
      std::ostream& out();

      // Marks the current test as "passed"
      // To call when we're done testing and the test
      // is successful.
      void passed();

      // Marks the current test as "failed"
      // To call when we're done testing and the test
      // has failed.
      void failed();
    private:
      std::ostream& out_;

      // Bitfield
      bool done_ : 1;
      bool passed_ : 1;
      // 6 bits left
  };

  /*
    This class is unique for each file. It contains
    instance variables and test preferences set by the file, and
    methods to execute the test file.
  */
  class FileTest: public Test
  {
    public:
      // Constructs a FileTest object that tests the file and outputs
      // diagnostics to os. (No test result is going to be printed to the stream,
      // only diagnostics/errors)
      FileTest(const std::string& file, std::ostream& os = std::cout);
  };
}
}