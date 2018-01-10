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
		LEX_log_pushed_token,		// <true/false> : the lexer will log whenever a token is pushed
		LEX_log_total_token_count	// <true/false>	: the lexer will log the total token found at the end of the lexing process
	};
	const std::map<OptionsList,std::string> k_OptionsStrNames =
	{
		{ OptionsList::TESTERCLASS_TESTOPT , "TESTERCLASS_TESTOPT" },
		{ OptionsList::LEX_log_pushed_token , "LEX_log_pushed_token" },
		{ OptionsList::LEX_log_total_token_count , "LEX_log_total_token_count"}
	};
	inline std::string getOptionName(const OptionsList& opt)
	{
		auto reqit = k_OptionsStrNames.find(opt);
		if (reqit != k_OptionsStrNames.end())
			return reqit->second;
		return "";
	}
}
