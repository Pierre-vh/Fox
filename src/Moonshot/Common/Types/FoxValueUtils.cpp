////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : FoxValueUtils.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "FoxValueUtils.hpp"

#include "Moonshot/Fox/Lexer/StringManipulator.hpp"
#include <map>
#include <sstream>

using namespace Moonshot;

const std::map<std::size_t, std::string> kBuiltinTypes_dict =
{
	{ TypeIndex::Void_Type			, "VOID" },
	{ TypeIndex::basic_Int			, "INT" },
	{ TypeIndex::basic_Float		, "FLOAT" },
	{ TypeIndex::basic_Char			, "CHAR" },
	{ TypeIndex::basic_Bool			, "BOOL" },
	{ TypeIndex::basic_String		, "STRING" },
	{ TypeIndex::InvalidIndex		, "!INVALID!" }
};

std::string FValUtils::dumpFVal(const FoxValue & fv)
{
	std::stringstream output;
	if (std::holds_alternative<VoidType>(fv))
		output << "Type : VOID (null)";
	else if (std::holds_alternative<IntType>(fv))
		output << "Type : INT, Value : " << std::get<IntType>(fv);
	else if (std::holds_alternative<float>(fv))
		output << "Type : FLOAT, Value : " << std::get<float>(fv);
	else if (std::holds_alternative<std::string>(fv))
		output << "Type : STRING, Value : \"" << std::get<std::string>(fv) << "\"";
	else if (std::holds_alternative<bool>(fv))
	{
		const bool v = std::get<bool>(fv);
		output << "Type : BOOL, Value : " << (v ? "true" : "false");
	}
	else if (std::holds_alternative<CharType>(fv))
	{
		CharType x = std::get<CharType>(fv);
		UTF8::StringManipulator u8sm;
		output << "Type : CHAR, Value : " << x << " = '" << u8sm.wcharToStr(x) << "'";
	}
	else
		throw std::logic_error("Illegal fviant.");
	return output.str();
}

std::string FValUtils::getTypenameForIndex(const std::size_t & ind)
{
	auto searchres = kBuiltinTypes_dict.find(ind);
	if (searchres != kBuiltinTypes_dict.end())
		return searchres->second;
	else
		throw std::invalid_argument("Unknown index in dictionary");
}
