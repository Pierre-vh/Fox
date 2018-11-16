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
#include <chrono>
#include "Fox/Common/string_view.hpp"

namespace fox {
  class Driver {
    public:
      // Verify mode: 
      //  Disabled: No diagnostic verification performed
      //  Soft: Diagnostic Verification is performed
      //  Normal: Same as basic, but additionally we'll check that
      //          every expected diagnostics were emitted.
      enum class VerifyMode {
        Disabled, Soft, Normal
      };

    private:
      VerifyMode verify_ = VerifyMode::Disabled;
      bool chrono_ = false;
      bool dumpAlloc_ = false;
      bool dumpAST_ = false;
      bool mute_ = false;
      std::ostream& os_;

    public:

      Driver(std::ostream& os);
      bool processFile(const std::string& filepath);

      bool getPrintChrono() const;
      void setPrintChrono(bool val);

      VerifyMode getVerifyMode() const;
      void setVerifyMode(VerifyMode val);

      bool getDumpAlloc() const;
      void setDumpAlloc(bool val);

      bool getDumpAST() const;
      void setDumpAST(bool val);

      std::ostream& getOS();

      bool doCL(int argc, char* argv[]);

    private:
      class RAIIChrono {
        public:
          Driver& driver;
          std::chrono::steady_clock::time_point beg;
          string_view label;
          RAIIChrono(Driver& driver, string_view label) :
            driver(driver), label(label) {
            if(driver.getPrintChrono())
              beg = std::chrono::high_resolution_clock::now();
          }

          ~RAIIChrono() {
            if (!driver.getPrintChrono()) return;
            auto end = std::chrono::high_resolution_clock::now();
            auto micro = std::chrono::duration_cast
              <std::chrono::microseconds>(end-beg).count();
            auto milli = std::chrono::duration_cast
              <std::chrono::milliseconds>(end-beg).count();
            driver.getOS() << label << " time:" << micro << " microseconds | "
              << milli << " milliseconds\n";
          }
      };

      RAIIChrono createChrono(string_view label);
  };
}
