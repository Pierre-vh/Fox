//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : DiagnosticConsumers.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the DiagnosticConsumer interface as well
// as some builtin implementations.
//----------------------------------------------------------------------------//

#pragma once

#include <iostream>

namespace fox {
  class Diagnostic;
  class SourceLoc;
  class SourceRange;
  class SourceManager;
  enum class DiagSeverity : std::uint8_t;

  class DiagnosticConsumer {
    public:
      virtual void consume(Diagnostic& diag) = 0;

    protected:
      std::string getLocInfo(SourceManager& sm, SourceRange range, bool isFileWide) const;
      std::string diagSevToString(DiagSeverity ds) const;

      // Removes the indentation (spaces and tabs) from a line, returning the number of indent chars removed
      std::size_t removeIndent(std::string& str) const;
  };

  class StreamDiagConsumer : public DiagnosticConsumer {
    public:
      StreamDiagConsumer(SourceManager& sm,std::ostream& stream = std::cout); // Default outstream is cout (stdio)
      virtual void consume(Diagnostic& diag) override;

    private:
      // Displays a line of code along with the caret.
      // Note: this only displays the first line where the problem begins.
      void displayRelevantExtract(const Diagnostic& diag);

      SourceManager& sm_;
      std::ostream &os_;
  };
}
