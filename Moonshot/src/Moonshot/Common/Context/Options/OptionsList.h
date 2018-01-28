////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : OptionsList.h											
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
		EXPRTEST_printAST,
		LEX_logPushedTokens,		// <true/false> : the lexer will log whenever a Token is pushed
		LEX_logTotalTokenCount,	// <true/false>	: the lexer will log the total Token found at the end of the lexing process
		DUMPER_useUTF8chars,
	};
	const std::map<OptionsList,std::string> k_OptionsStrNames =
	{
		{ OptionsList::TESTERCLASS_TESTOPT , "TESTERCLASS_TESTOPT" },
		{ OptionsList::EXPRTEST_printAST , "EXPRTEST_printAST,"},
		{ OptionsList::LEX_logPushedTokens , "LEX_logPushedTokens" },
		{ OptionsList::LEX_logTotalTokenCount , "LEX_logTotalTokenCount"},
		{ OptionsList::DUMPER_useUTF8chars, "DUMPER_useUTF8chars"}
	};
	inline std::string getOptionName(const OptionsList& opt)
	{
		auto reqit = k_OptionsStrNames.find(opt);
		if (reqit != k_OptionsStrNames.end())
			return reqit->second;
		return "";
	}
}
