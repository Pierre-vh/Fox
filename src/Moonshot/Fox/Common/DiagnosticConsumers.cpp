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

using namespace Moonshot;

StreamDiagConsumer::StreamDiagConsumer(SourceManager *sm, std::ostream & stream) : os_(stream), sm_(sm)
{

}

void StreamDiagConsumer::consume(const Diagnostic& diag)
{
	os_ << getLocInfo(diag.getSourceRange().getBeginSourceLoc()) 
		<< " - " 
		<< diagSevToString(diag.getDiagSeverity()) 
		<< " - " 
		<< diag.getDiagStr() 
		<< "\n";
}

std::string StreamDiagConsumer::getLocInfo(const SourceLoc& loc) const
{
	if (!loc || !sm_)
		return "<unknown>";

	CompleteLoc compLoc = sm_->getCompleteLocForSourceLoc(loc);

	std::stringstream ss;
	ss << "<" << compLoc.fileName << ">(l:" << compLoc.line << ",c:" << compLoc.column << ")";
	return ss.str();
}

std::string StreamDiagConsumer::diagSevToString(const DiagSeverity & ds) const
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
