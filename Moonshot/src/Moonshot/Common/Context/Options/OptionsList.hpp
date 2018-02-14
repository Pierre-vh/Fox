////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : OptionsList.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This class defines the enum "OptionsList"							
////------------------------------------------------------////

#pragma once

#include <map>

namespace Moonshot
{
	enum class OptionsList
	{
		TESTERCLASS_TESTOPT,
		exprtest_printAST,
		lexer_logPushedTokens,		// <true/false> : the lexer will log whenever a Token is pushed
		lexer_logTotalTokenCount,	// <true/false>	: the lexer will log the total Token found at the end of the lexing process
		astdump_useUTF8chars,
		parser_maxExpectedErrorCount,
		parser_printSuggestions
	};
	namespace Dicts
	{
		const std::map<OptionsList, std::string> k_OptionsStrNames =
		{
			{ OptionsList::TESTERCLASS_TESTOPT , "TESTERCLASS_TESTOPT" },
			{ OptionsList::exprtest_printAST , "exprtest_printAST," },
			{ OptionsList::lexer_logPushedTokens , "lexer_logPushedTokens" },
			{ OptionsList::lexer_logTotalTokenCount , "lexer_logTotalTokenCount" },
			{ OptionsList::astdump_useUTF8chars, "astdump_useUTF8chars" },
			{ OptionsList::parser_maxExpectedErrorCount, "parser_maxExpectedErrorCount" },
			{ OptionsList::parser_printSuggestions,"parser_printSuggestions" }
		};
	}
	inline std::string getOptionName(const OptionsList& opt)
	{
		auto reqit = Dicts::k_OptionsStrNames.find(opt);
		if (reqit != Dicts::k_OptionsStrNames.end())
			return reqit->second;
		return "";
	}
}
