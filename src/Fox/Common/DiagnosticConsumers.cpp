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
#include <cassert>
#include <string>
#include <sstream>

using namespace fox;

std::string DiagnosticConsumer::getLocInfo(SourceManager* sm, const SourceRange& range, bool isFileWide) const
{
	if (!range || !sm)
		return "<unknown>";

	CompleteLoc beg = sm->getCompleteLocForSourceLoc(range.getBeginSourceLoc());

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
		CompleteLoc end = sm->getCompleteLocForSourceLoc(range.makeEndSourceLoc());
		ss << "-" << end.column;
	}
	return ss.str();
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


StreamDiagConsumer::StreamDiagConsumer(SourceManager *sm, std::ostream & stream) : os_(stream), sm_(sm)
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

	// todo: display line without indent
	if (sm_ && !diag.isFileWide())
	{
		os_ << sm_->getLineAtLoc(diag.getSourceRange().getBeginSourceLoc()) << "\n";
	}
}