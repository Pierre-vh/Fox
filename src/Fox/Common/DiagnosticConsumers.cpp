////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : DiagnosticConsumers.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "DiagnosticConsumers.hpp"
#include "Diagnostic.hpp"
#include "utfcpp/utf8.hpp"
#include <cassert>
#include <string>
#include <sstream>

using namespace fox;

std::string DiagnosticConsumer::getLocInfo(SourceManager& sm, const SourceRange& range, bool isFileWide) const
{
	if (!range)
		return "<unknown>";

	CompleteLoc beg = sm.getCompleteLocForSourceLoc(range.getBegin());

	std::stringstream ss;
	ss << "<" << beg.fileName << '>';

	// Don't display the column/line for file wide diags
	if (!isFileWide)
		ss << ':' << beg.line << ':' << beg.column;

	// A better approach (read: a faster approach) 
	// would be to have a special method in the SourceManager calculating the preciseLoc
	// for a SourceRange (so we avoid calling "getCompleteLocForSourceLoc" twice)
	if (range.getOffset() != 0)
	{
		CompleteLoc end = sm.getCompleteLocForSourceLoc(range.getEnd());
		ss << "-" << end.column;
	}
	return ss.str();
}

std::size_t DiagnosticConsumer::removeIndent(std::string& str) const
{
	std::size_t indent = 0;
	// Get number of char that are spaces/tabs at the beginning of the line
	for (auto it = str.begin(); it != str.end(); str.erase(0, 1))
	{
		if ((*it == ' ') || (*it == '\t'))
			indent++;
		else
			break;
	}

	// Erase at the end
	for (auto it = str.rbegin(); it != str.rend(); it++)
	{
		if ((*it == ' ') || (*it == '\t'))
			str.pop_back();
		else
			break;
	}

	return indent;
}

std::string DiagnosticConsumer::diagSevToString(DiagSeverity ds) const
{
	switch (ds)
	{
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


StreamDiagConsumer::StreamDiagConsumer(SourceManager &sm, std::ostream & stream) : os_(stream), sm_(sm)
{

}

void StreamDiagConsumer::consume(const Diagnostic& diag)
{
	os_ << getLocInfo(sm_, diag.getSourceRange(), diag.isFileWide())
		<< " - " 
		<< diagSevToString(diag.getDiagSeverity()) 
		<< " - " 
		<< diag.getDiagStr() 
		<< "\n";

	if (!diag.isFileWide())
		displayRelevantExtract(diag);
}

void StreamDiagConsumer::displayRelevantExtract(const Diagnostic& diag)
{
	SourceLoc::idx_type lineBeg = 0;
	SourceLoc begLoc = diag.getSourceRange().getBegin();
	SourceLoc endLoc = diag.getSourceRange().getEnd();

	// Get the line, remove it's indent and display it.
	std::string line = sm_.getLineAtLoc(begLoc, &lineBeg);
	auto lineBegNoIndent =  lineBeg + removeIndent(line);
	os_ << '\t' << line << '\n';

	auto bytesBeforeCaret = begLoc.getIndex() - lineBegNoIndent;

	std::string caretLine = "";

	// Prepare some handy iterators
	auto strBeg = line.begin();
	auto caretBeg = strBeg + bytesBeforeCaret;
	auto caretEnd = caretBeg + diag.getSourceRange().getOffset();

	// Add spaces
	std::size_t spacesBeforeCaret = utf8::distance(strBeg, caretBeg);
	for (std::size_t k = 0; k < spacesBeforeCaret; k++)
		caretLine += ' ';

	// Add carets
	std::size_t numCarets = 1 + utf8::distance(caretBeg, caretEnd);
	for (std::size_t k = 0; k < numCarets; k++)
	{
		// Stop generating carets if the caretLine is longer
		// than the line's size + 1 (to allow a caret at the end of the line)
		if (caretLine.size() > (line.size() + 1))
			break;
		caretLine += '^';
	}


	// Display the caret's line.
	os_ << '\t' << caretLine << '\n';
}