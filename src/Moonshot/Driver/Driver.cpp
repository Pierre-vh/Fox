////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Driver.cpp											
// Author : Pierre van Houtryve											
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Driver.hpp"

#include "Moonshot/Fox/Common/Context.hpp"
#include "Moonshot/Fox/Lexer/Lexer.hpp"
#include "Moonshot/Fox/Parser/Parser.hpp"
#include "Moonshot/Fox/AST/ASTDumper.hpp"
#include "Moonshot/Fox/AST/ASTContext.hpp"

#include <fstream>

using namespace Moonshot;

bool Driver::processFile(std::ostream& out, const std::string& filepath)
{
	Context ctxt(Context::LoggingMode::SAVE_TO_VECTOR);
	// Create a ASTContext
	ASTContext astCtxt;

	auto fid = ctxt.sourceManager.loadFromFile(filepath);
	if (!fid)
	{
		std::cout << "Could not open file \"" << filepath << "\"\n";
		return false;
	}

	Lexer lex(ctxt,astCtxt);
	lex.lexFile(fid);

	if (!ctxt.isSafe())
	{
		out << "Failed at lexing. Logs:\n";
		out << ctxt.getLogs();
		return false;
	}
	else
		out << "Lexing completed successfully." << lex.getTokenVector().size() << " tokens found.\n";

	Parser psr(ctxt,astCtxt,lex.getTokenVector());
	// Todo: extract the name of the file and use that instead of "TestUnit"
	auto unit = psr.parseUnit(fid,astCtxt.identifiers.getUniqueIdentifierInfo("TestUnit"));

	out << ctxt.getLogs();
	if (!unit)
	{
		out << "Failed at parsing.";
		return false;
	}
	else
		out << "Parsing successful!\n";

	
	// set as main unit
	astCtxt.setMainUnit(unit.move());

	out << "\nMain Unit Dump:\n";
	
	ASTDumper dumper(ctxt,std::cout,1);
	dumper.visit(astCtxt.getMainUnit());
	
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
