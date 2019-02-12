//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : DiagnosticConsumers.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the DiagnosticConsumer interface as well
// as some builtin implementations.
//----------------------------------------------------------------------------//

#pragma once

#include <iosfwd>
#include "string_view.hpp"

namespace fox {
  class Diagnostic;
  class SourceLoc;
  class SourceRange;
  class SourceManager;
  enum class DiagSeverity : std::uint8_t;

  class DiagnosticConsumer {
    public:
      virtual void consume(SourceManager& sm, const Diagnostic& diag) = 0;
      virtual ~DiagnosticConsumer() = default;

    protected:
      std::string getLocInfo(SourceManager& sm, SourceRange range, 
        bool isFileWide) const;

      // Removes the indentation (spaces and tabs) from a string_view, 
      // returning the number of indent chars removed
      std::size_t removeIndent(string_view& str) const;
  };

  class StreamDiagConsumer : public DiagnosticConsumer {
    public:
      StreamDiagConsumer(std::ostream& stream); 
      StreamDiagConsumer(); 

      virtual void consume(SourceManager& sm, const Diagnostic& diag) override;

    private:
      // Displays a line of code along with the caret.
      // Note: this only displays the first line where the problem begins.
      void displayRelevantExtract(SourceManager& sm, const Diagnostic& diag);

      std::ostream &os_;
  };
}
