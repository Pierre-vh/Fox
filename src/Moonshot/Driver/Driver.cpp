////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Driver.cpp											
// Author : Pierre van Houtryve											
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Driver.hpp"

#include "Moonshot/Fox/Basic/Context.hpp"
#include "Moonshot/Fox/Lexer/Lexer.hpp"
#include "Moonshot/Fox/Parser/Parser.hpp"
#include "Moonshot/Fox/AST/Dumper.hpp"
#include "Moonshot/Fox/AST/ASTContext.hpp"

#include <fstream>

using namespace Moonshot;

bool Driver::compileFunction(std::ostream& out, const std::string& filepath)
{
	Context ctxt(Context::LoggingMode::SAVE_TO_VECTOR);
	// Create a ASTContext
	ASTContext astCtxt;

	std::string filecontent;
	if (!readFileToString(filepath, filecontent))
	{
		out << "Could not open file \"" << filepath << "\"\n";
		return false;
	}

	Lexer lex(ctxt,astCtxt);
	lex.lexStr(filecontent);

	if (!ctxt.isSafe())
	{
		out << "Failed at lexing. Logs:\n";
		out << ctxt.getLogs();
		return false;
	}
	else
		out << "Lexing completed successfully." << lex.getTokenVector().size() << " tokens found.\n";

	Parser psr(ctxt,astCtxt,lex.getTokenVector());
	auto presult = psr.parseUnit();

	out << ctxt.getLogs();
	if (!presult)
	{
		out << "Failed at parsing.";
		if (!presult) // no usable data? return now.
		{
			out << "Failed to parse unit.\n";
			return false;
		}
	}
	else
		out << "Parsing successful!\n";

	
	// set as main unit
	astCtxt.setMainUnit(std::move(presult.unit));

	out << "\nMain Unit Dump:\n";
	Dumper dump(out,1);
	dump.dumpUnit(*(astCtxt.getMainUnit()));
	return true;
}

bool Driver::readFileToString(const std::string & filepath, std::string & outstr) const
{
	std::ifstream in(filepath, std::ios::binary); 	// read file
	if (in)
	{
		outstr = (std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()));
		return true;
	}
	return false;
}
