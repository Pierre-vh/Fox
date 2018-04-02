////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Driver.cpp											
// Author : Pierre van Houtryve											
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Driver.hpp"

#include "Moonshot/Common/Context/Context.hpp"
#include "Moonshot/Fox/Lexer/Lexer.hpp"
#include "Moonshot/Fox/Parser/Parser.hpp"
#include "Moonshot/Fox/AST/Dumper.hpp"

#include <fstream>

using namespace Moonshot;

bool Driver::compileFunction(std::ostream& out, const std::string& filepath)
{
	Context ctxt(Context::LoggingMode::SAVE_TO_VECTOR);
	std::string filecontent;
	if (!readFileToString(filepath, filecontent))
	{
		out << "Could not open file \"" << filepath << "\"\n";
		return false;
	}

	Lexer lex(ctxt);
	lex.lexStr(filecontent);

	if (!ctxt.isSafe())
	{
		out << "Failed at lexing. Logs:\n";
		out << ctxt.getLogs();
		return false;
	}

	Parser psr(ctxt,lex.getTokenVector());
	auto presult = psr.parseFunctionDeclaration();

	if (!presult || (presult.getFlag() == ParsingOutcome::NOTFOUND) || !ctxt.isSafe())
	{
		out << "Failed at parsing. Logs:\n";
		out << ctxt.getLogs();
		return false;
	}

	out << "Success ! AST:\n";
	auto node = std::move(presult.result_);
	Dumper dump(out,1);
	node->accept(dump);
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
