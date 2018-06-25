////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Driver.cpp											
// Author : Pierre van Houtryve											
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Driver.hpp"

#include "Fox/Common/Context.hpp"
#include "Fox/Lexer/Lexer.hpp"
#include "Fox/Parser/Parser.hpp"
#include "Fox/AST/ASTDumper.hpp"
#include "Fox/AST/ASTContext.hpp"
#include <iostream>
#include <chrono>
#include <fstream>

using namespace fox;

bool Driver::processFile(std::ostream& out, const std::string& filepath)
{
	Context ctxt(Context::LoggingMode::SAVE_TO_VECTOR);
	DiagnosticEngine dg(&ctxt.sourceManager);
	// Create a ASTContext
	std::unique_ptr<ASTContext> astCtxt = std::make_unique<ASTContext>();

	auto fid = ctxt.sourceManager.loadFromFile(filepath);
	if (!fid)
	{
		std::cout << "Could not open file \"" << filepath << "\"\n";
		return false;
	}
	auto t0 = std::chrono::high_resolution_clock::now();

	Lexer lex(dg,ctxt.sourceManager,*astCtxt);
	lex.lexFile(fid);

	auto t1 = std::chrono::high_resolution_clock::now();
	auto lex_micro = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
	auto lex_milli = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

	if (!ctxt.isSafe())
	{
		out << "Failed at lexing. Logs:\n";
		out << ctxt.getLogs();
		return false;
	}
	else
		out << "Lexing completed successfully." << lex.getTokenVector().size() << " tokens found.\n";

	Parser psr(ctxt,*astCtxt,lex.getTokenVector());
	// Todo: extract the name of the file and use that instead of "TestUnit"
	auto unit = psr.parseUnit(fid,astCtxt->identifiers.getUniqueIdentifierInfo("TestUnit"), /* is main unit */ true);

	out << ctxt.getLogs();
	if (!unit)
	{
		out << "Failed at parsing.";
		return false;
	}
	else
		out << "Parsing successful!\n";

	out << "\nMain Unit Dump:\n";
	
	auto t2 = std::chrono::high_resolution_clock::now();
	auto parse_micro = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
	auto parse_milli = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
	
	ASTDumper dumper(ctxt, std::cout, 1);
	dumper.visit(astCtxt->getMainUnit());

	auto t3 = std::chrono::high_resolution_clock::now();
	auto dump_micro = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();
	auto dump_milli = std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count();

	astCtxt.reset();

	auto t4 = std::chrono::high_resolution_clock::now();
	auto release_micro = std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count();
	auto release_milli = std::chrono::duration_cast<std::chrono::milliseconds>(t4 - t3).count();

	std::cout << "\nLexing time :\n\t" << lex_micro << " microseconds\n\t" << lex_milli << " milliseconds\n";
	std::cout << "\nParsing time :\n\t" << parse_micro << " microseconds\n\t" << parse_milli << " milliseconds\n";
	std::cout << "\nAST dump time :\n\t" << dump_micro << " microseconds\n\t" << dump_milli << " milliseconds\n";
	std::cout << "\nAST release time :\n\t" << release_micro << " microseconds\n\t" << release_milli << " milliseconds\n";

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
