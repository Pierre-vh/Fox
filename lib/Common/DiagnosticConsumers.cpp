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

std::string 
DiagnosticConsumer::getLocInfo(SourceManager& sm, SourceRange range, 
                               bool isFileWide) const {
  assert(range && "invalid range");
  std::stringstream ss;
  // Only display the file name for file-wide diagnostics
  if (isFileWide)
    ss << sm.getFileName(range.getFileID());
  else {
    CompleteLoc full = sm.getCompleteLoc(range.getBeginLoc());
    ss << full.fileName << ':' << full.line << ':' << full.column;
  }
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
  if (SourceRange range = diag.getSourceRange())
    os_ << getLocInfo(sm, diag.getSourceRange(), diag.isFileWide()) << ": ";
  os_ << toString(diag.getSeverity()) 
    << ": " 
    << diag.getStr() 
    << "\n";

  // If the Diagnostic contains valid location information, and it
  // isn't a file-wide diagnostic, display a snippet (a single line)
  // of the source file with the Diagnostic message.
  if (diag.hasRange()) displayRelevantExtract(sm, diag);
}

// Helper method for "displayRelevantExtract" that creates the "underline" string. 
std::string createUnderline(char underlineChar, std::size_t beg, std::size_t end) {
  std::string line = "";

  for (std::size_t k = 0; k < beg; k++)
    line += ' ';

  for (std::size_t k = beg; k < end; k++)
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

// Returns true if the caret line should be printed, false otherwise.
//
// In short, we won't print the caret line if the source extract contains
// non-ascii characters, because having proper unicode support is really
// hard. Some characters might be wider (full width chars, e.g. japanese ones)
// than ascii ones, messing up the whole formatting.
//
// This is sort of a FIXME. One day, supporting non ascii characters
// would be a great feature!
static bool shouldPrintCaretLine(string_view sourceExtract) {
  for (unsigned char byte : sourceExtract) {
    // Don't the caret line for non ascii chars.
    if (byte & 0x80) return false;
  }
  return true;
}

void StreamDiagConsumer::displayRelevantExtract(SourceManager& sm, 
  const Diagnostic& diag) {
  assert(diag.hasRange() 
		&& "Cannot use this if the diag does not have SourceRange!");

  auto range = diag.getSourceRange();
  auto eRange = diag.getExtraRange();

  // Get the sourceLine
  SourceLoc lineBeg;
  string_view sourceLine = 
    sm.getLineAt(diag.getSourceRange().getBeginLoc(), &lineBeg);
  std::size_t lineSize = utf8::distance(sourceLine.begin(), sourceLine.end());

  // Remove any indent, and offset the linebeg loc accordingly.
  std::size_t offset = removeIndent(sourceLine);
  lineBeg = sm.incrementSourceLoc(lineBeg, offset);

  // Display the source extract
  os_ << "    " << sourceLine << '\n';

  // Check if we must print the caret line. If we don't need
  // to print it, we're done.
  if(!shouldPrintCaretLine(sourceLine)) return;

  std::string underline;
  // Create the carets underline (^)
	{  
    SourceRange preRange(lineBeg, range.getBeginLoc());
    // We'll begin the range at the last codepoint, so uBeg is
    // the number of codepoints in the range minus one.
    auto uBeg = sm.getLengthInCodepoints(preRange)-1;
    // Change the beginning of the range so it begins where the sourceLine
    // begins.
    SourceRange rangeInLine = SourceRange(lineBeg, range.getEndLoc());
    // Calculate the number of codepoints in that range
    std::size_t uEnd = sm.getLengthInCodepoints(rangeInLine);
    // But check that the number doesn't exceed sourceLine's size.
    uEnd = std::min(uEnd, lineSize+1);
    underline = createUnderline('^', uBeg, uEnd);
  }

  // If needed, create the extra range underline (~)
  if(diag.hasExtraRange()) {
    assert((diag.getExtraRange().getFileID() == 
            diag.getSourceRange().getFileID())
      && "Ranges don't belong to the same file");

    SourceRange preRange(lineBeg, eRange.getBeginLoc());
    // We'll begin the range at the last codepoint, so uBeg is
    // the number of codepoints in the range minus one.
    auto uBeg = sm.getLengthInCodepoints(preRange)-1;

    // Change the beginning of the range so it begins where the sourceLine
    // begins.
    SourceRange rangeInLine = SourceRange(lineBeg, eRange.getEndLoc());
    // Calculate the number of codepoints in that range
    std::size_t uEnd = sm.getLengthInCodepoints(rangeInLine);
    // But check that the number doesn't exceed sourceLine's size.
    uEnd = std::min(uEnd, lineSize+1);
    underline = embedString(underline, createUnderline('~', uBeg, uEnd));
  }

  // Display the carets
  os_ << "    " << underline << '\n';
}
