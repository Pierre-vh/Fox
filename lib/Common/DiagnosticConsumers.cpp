//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : DiagnosticConsumers.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Common/DiagnosticConsumers.hpp"
#include "Fox/Common/Diagnostic.hpp"
#include "utfcpp/utf8.hpp"
#include <cassert>
#include <string>
#include <sstream>

using namespace fox;

std::string DiagnosticConsumer::getLocInfo(SourceManager& sm, SourceRange range, bool isFileWide) const {
  if (!range)
    return "<unknown>";

  CompleteLoc beg = sm.getCompleteLoc(range.getBegin());

  std::stringstream ss;
  ss << "<" << beg.fileName << '>';

  // Don't display the column/line for file wide diags
  if (!isFileWide)
    ss << ':' << beg.line << ':' << beg.column;

  // A better approach (read: a faster approach) 
  // would be to have a special method in the SourceManager calculating the preciseLoc
  // for a SourceRange (so we avoid calling "getCompleteLocForSourceLoc" twice)
  if (range.getOffset() != 0) {
    CompleteLoc end = sm.getCompleteLoc(range.getEnd());
    ss << "-" << end.column;
  }
  return ss.str();
}

std::size_t DiagnosticConsumer::removeIndent(string_view& str) const {
  std::size_t beg = 0,
              end = str.size()-1;

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

std::string DiagnosticConsumer::diagSevToString(DiagSeverity ds) const {
  switch (ds) {
    case DiagSeverity::IGNORE:
      return "Ignored";
    case DiagSeverity::NOTE:
      return "Note";
    case DiagSeverity::WARNING:
      return "Warning";
    case DiagSeverity::ERROR:
      return "Error";
    case DiagSeverity::FATAL:
      return "Fatal";
  }
  return "<Unknown Severity>";
}


StreamDiagConsumer::StreamDiagConsumer(SourceManager &sm, std::ostream & stream) : os_(stream), sm_(sm) {

}

void StreamDiagConsumer::consume(Diagnostic& diag) {
  os_ << getLocInfo(sm_, diag.getRange(), diag.isFileWide())
    << " - " 
    << diagSevToString(diag.getDiagSeverity()) 
    << " - " 
    << diag.getDiagStr() 
    << "\n";

  if (!diag.isFileWide() && diag.hasRange())
    displayRelevantExtract(diag);
}

// Helper method for "displayRelevantExtract" which creates the "underline" string. 
// The beg/end args represent the range in the string where the underline should be.
std::string createUnderline(char underlineChar, string_view str, 
  string_view::const_iterator beg, string_view::const_iterator end) {
  std::string line = "";

  auto strBeg = str.begin();

  // Calculate the number of spaces before the caret and add them
  std::size_t spacesBeforeCaret = utf8::distance(strBeg, beg);

  for (std::size_t k = 0; k < spacesBeforeCaret; k++)
    line += ' ';

  // Calculate the number fo carets we need
  std::size_t numCarets = 1 + utf8::distance(beg, end);
  for (std::size_t k = 0; k < numCarets; k++)
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

void StreamDiagConsumer::displayRelevantExtract(const Diagnostic& diag) {
  assert(diag.hasRange() && "Cannot use this if the diag does not have a valid range");

  auto range = diag.getRange();
  auto eRange = diag.getExtraRange();
  SourceLoc::IndexTy lineBeg = 0;

  // Get the line, remove it's indent and display it.
  string_view line = sm_.getSourceLine(diag.getRange().getBegin(), &lineBeg);

  // Remove any indent, and offset the lineBeg accordingly
  lineBeg += removeIndent(line);

  std::string underline;

  auto getOffsetIteratorFromLineBeg = [&](std::size_t idx) {
    auto result = idx - lineBeg;
    if (result > line.size())
      result = line.size() - 1;
    return line.begin() + result;
  };

  // Create the carets underline (^)
	{  
    auto beg = getOffsetIteratorFromLineBeg(range.getBegin().getIndex());
    auto end = getOffsetIteratorFromLineBeg(range.getEnd().getIndex());
    underline = createUnderline('^', line, beg, end);
  }

  // If needed, create the extra range underline (~)
  if(diag.hasExtraRange()) {
    assert((diag.getExtraRange().getFileID() == diag.getRange().getFileID())
      && "Ranges don't belong to the same file");

    auto beg = getOffsetIteratorFromLineBeg(eRange.getBegin().getIndex());
    auto end = getOffsetIteratorFromLineBeg(eRange.getEnd().getIndex());
    underline = embedString(underline, createUnderline('~', line, beg, end));
  }

  // Display the line
  os_ << '\t' << line << '\n';
  // Display the carets
  os_ << '\t' << underline << '\n';
}
