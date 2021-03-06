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
#include <ostream>
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

StreamDiagConsumer::StreamDiagConsumer(std::ostream& stream):
  os_(stream) {}

void StreamDiagConsumer::consume(SourceManager& sm, const Diagnostic& diag) {
  if (SourceRange range = diag.getSourceRange())
    os_ << getLocInfo(sm, diag.getSourceRange(), diag.isFileWide()) << ": ";
  os_ << to_string(diag.getSeverity()) 
    << ": " 
    << diag.getStr() 
    << "\n";

  // If the Diagnostic contains valid location information, and it
  // isn't a file-wide diagnostic, display a snippet (a single line)
  // of the source file with the Diagnostic message.
  if (diag.hasPreciseLoc()) showRelevantSnippet(sm, diag);
}

// Helper method for "showRelevantSnippet" that creates the "underline" string. 
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
// FIXME: One day, supporting non ascii characters would be a great feature!
static bool shouldPrintCaretLine(string_view sourceExtract) {
  for (unsigned char byte : sourceExtract) {
    // Don't the caret line for non ascii chars.
    if (byte & 0x80) return false;
  }
  return true;
}

void StreamDiagConsumer::showRelevantSnippet(SourceManager& sm, 
                                             const Diagnostic& diag) {
  auto range = diag.getSourceRange();
  auto eRange = diag.getExtraRange();

  // Get the sourceLine
  SourceLoc lineBeg;
  string_view sourceLine = 
    sm.getLineAt(diag.getSourceRange().getBeginLoc(), &lineBeg);

  // Remove any indent, and offset the linebeg loc accordingly.
  std::size_t offset = removeIndent(sourceLine);
  lineBeg = sm.advance(lineBeg, offset);

  // Calculate the size of the line
  std::size_t lineSize = utf8::distance(sourceLine.begin(), sourceLine.end());

  // Display the source extract
  os_ << "    " << sourceLine << '\n';

  // Check if we must print the caret line. If we don't need
  // to print it, we're done.
  if(!shouldPrintCaretLine(sourceLine)) return;

  // This Checks that len isn't greater than the size of the line plus one.
  // If it is, is sets it to lineSize.
  // NOTE: We only allow that uEnd is in excess of 1 because that means
  //  it's simply an 'out-of-range' diag, like those emitted by the parser
  //  when a semicolon is missing. When it's in excess by more than one, it
  //  usually means that the primary range of the diagnostic spans multiple
  //  lines, and we don't want to have a past-the-end underline in that
  //  scenario.
  auto getUEnd = [&](std::size_t len) {
    if(len > (lineSize+1))
      return lineSize;
    return len;
  };

  std::string underline;

  // Underline the primary range with carets
	{  
    SourceRange preRange(lineBeg, range.getBeginLoc());
    // We'll begin the range at the last codepoint, so uBeg is
    // the number of codepoints in the range minus one.
    auto uBeg = sm.getLengthInCodepoints(preRange)-1;
    // Create a range that starts at the beginning of the line and
    // ends at the end of the range.
    SourceRange rangeInLine = SourceRange(lineBeg, range.getEndLoc());
    // Calculate uEnd using that.
    std::size_t uEnd = getUEnd(sm.getLengthInCodepoints(rangeInLine));
    // Create the underline
    underline = createUnderline('^', uBeg, uEnd);
  }

  // Underline the extra range with tildes
  if(diag.hasExtraLoc()) {
    assert((diag.getExtraRange().getFileID() == 
            diag.getSourceRange().getFileID())
      && "Ranges don't belong to the same file");

    SourceRange preRange(lineBeg, eRange.getBeginLoc());
    // We'll begin the range at the last codepoint, so uBeg is
    // the number of codepoints in the range minus one.
    auto uBeg = sm.getLengthInCodepoints(preRange)-1;
    // Create a range that starts at the beginning of the line and
    // ends at the end of the range.
    SourceRange rangeInLine = SourceRange(lineBeg, eRange.getEndLoc());
    // Calculate uEnd using that.
    std::size_t uEnd = getUEnd(sm.getLengthInCodepoints(rangeInLine));
    // Create the underline
    underline = embedString(underline, createUnderline('~', uBeg, uEnd));
  }

  // Print the underline
  os_ << "    " << underline << '\n';
}
