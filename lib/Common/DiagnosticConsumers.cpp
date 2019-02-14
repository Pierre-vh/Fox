//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : DiagnosticConsumers.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Common/DiagnosticConsumers.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"
#include "Fox/Common/SourceManager.hpp"
#include "utfcpp/utf8.hpp"
#include <cassert>
#include <string>
#include <iostream>
#include <sstream>

using namespace fox;

std::string DiagnosticConsumer::getLocInfo(SourceManager& sm, 
	SourceRange range, bool isFileWide) const {
  // TODO: Once I have something that resembles a "Project name" or "module name"
  // return that instead of an empty string so we have better diag handling
  // in that situation.
  // e.g. print "<MyModule> - error - ..." instead of just "error - ...."

  // Don't display anything if the range isn't valid.
  if (!range) return "";
  std::stringstream ss;
  // Only display the file name for file-wide diagnostics
  if (isFileWide)
    ss << '<' << sm.getFileName(range.getFileID()) << ">";
  else 
    ss << sm.getCompleteRange(range).toString();
  return ss.str();
}

std::size_t DiagnosticConsumer::removeIndent(string_view& str) const {
  std::size_t beg = 0, end = str.size();

  // Determine where the substring should begin
  for (auto it = str.begin(); it != str.end(); it++) {
    if ((*it == ' ') || (*it == '\t'))
      beg++;
    else break;
  }

  // Determine where the substring should end.
  for (auto it = str.rbegin(); it != str.rend(); it++) {
    if ((*it == ' ') || (*it == '\t'))
      end--;
    else break;
  }
  str = str.substr(beg, end-beg);
  // Beg = the number of indent char we just removed.
  return beg;
}

StreamDiagConsumer::StreamDiagConsumer(std::ostream & stream):
  os_(stream) {}

StreamDiagConsumer::StreamDiagConsumer() : StreamDiagConsumer(std::cout) {}

void StreamDiagConsumer::consume(SourceManager& sm, const Diagnostic& diag) {
  std::string locInfo = getLocInfo(sm, diag.getRange(), diag.isFileWide());
  if (locInfo.size())
    os_ << locInfo << " - ";
  os_ << toString(diag.getSeverity()) 
    << " - " 
    << diag.getStr() 
    << "\n";

  // If the Diagnostic contains valid location information, and it
  // isn't a file-wide diagnostic, display a snippet (a single line)
  // of the source file with the Diagnostic message.
  if (diag.hasRange()) displayRelevantExtract(sm, diag);
}

// Helper method for "displayRelevantExtract" which creates the "underline" string. 
// The "underline" begins at beg, and ends at "end". (it's a closed interval) 
std::string createUnderline(char underlineChar, std::size_t beg, std::size_t end) {
  std::string line = "";

  for (std::size_t k = 0; k < beg; k++)
    line += ' ';

  for (std::size_t k = beg; k <= end; k++)
    line += underlineChar;

  return line;
}

// Embeds "b" into "a", meaning that every space in a will be replaced with
// the character at the same position in b, and returns the string
// Example: embed("  ^  ", " ~~~ ") returns " ~^~ "
std::string embedString(const std::string& a, const std::string& b) {
  std::string out;
  for (std::size_t k = 0, sz = a.size(); k < sz; k++) {
    if ((a[k] == ' ') && (k < b.size())) {
      out += b[k];
      continue;
    }
    out += a[k];
  }

  if (b.size() > a.size()) {
    for (std::size_t k = a.size(); k < b.size(); k++)
      out += b[k];
  }

  return out;
}

void StreamDiagConsumer::displayRelevantExtract(SourceManager& sm, 
  const Diagnostic& diag) {
  assert(diag.hasRange() 
		&& "Cannot use this if the diag does not have SourceRange!");

  auto range = diag.getRange();
  auto eRange = diag.getExtraRange();

  // Get the line
  SourceLoc lineBeg;
  string_view line = sm.getLineAt(diag.getRange().getBegin(), &lineBeg);
  std::size_t lineSize = utf8::distance(line.begin(), line.end());

  // Remove any indent, and offset the linebeg loc accordingly.
  std::size_t offset = removeIndent(line);
  lineBeg = sm.incrementSourceLoc(lineBeg, offset);

  std::string underline;

  // Create the carets underline (^)
	{  
    auto uBeg = sm.getDifference(lineBeg, range.getBegin());
    auto uEnd = std::min(sm.getDifference(lineBeg, range.getEnd()), lineSize);
    underline = createUnderline('^', uBeg, uEnd);
  }

  // If needed, create the extra range underline (~)
  if(diag.hasExtraRange()) {
    assert((diag.getExtraRange().getFileID() == diag.getRange().getFileID())
      && "Ranges don't belong to the same file");

    auto uBeg = sm.getDifference(lineBeg, eRange.getBegin());
    auto uEnd = std::min(sm.getDifference(lineBeg, eRange.getEnd())+1, lineSize);
    underline = embedString(underline, createUnderline('~', uBeg, uEnd));
  }

  // Display the line
  os_ << '\t' << line << '\n';
  // Display the carets
  os_ << '\t' << underline << '\n';
}
