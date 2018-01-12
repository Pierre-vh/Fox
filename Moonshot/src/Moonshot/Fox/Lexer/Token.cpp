////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Token.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Token.h"
using namespace Moonshot;


token::token(Context & c) : context_(c)
{
	empty_ = true;
}
token::token(Context & c,std::string data, const text_pos &tpos) : context_(c),str(data),pos(tpos)
{
	idToken(); // self id
}

std::string token::showFormattedTokenData() const
{
	if (empty_) // Token is empty
		return "<EMPTY TOKEN>"; // return nothing.

	std::stringstream ss;
	ss << "[str:\"" << str << "\"][" << pos.asText() << "][type:";
	int enum_info = -1;		// The information of the corresponding enumeration
	switch (type)
	{
		case tokenType::TT_ENUM_DEFAULT:
			ss << "ENUM_DEFAULT";
			break;
		case tokenType::TT_IDENTIFIER:
			ss << "IDENTIFIER";
			enum_info = -2;
			break;
		case tokenType::TT_KEYWORD:
			ss << "KEYWORD";
			enum_info = util::enumAsInt(kw_type);
			break;
		case tokenType::TT_SIGN:
			ss << "SIGN";
			enum_info = util::enumAsInt(sign_type);
			break;
		case tokenType::TT_LITERAL:
			ss << "VALUE";
			enum_info = util::enumAsInt(val_type);
			break;
	}
	if (enum_info >= -1)
		ss << " -> E:" << enum_info;
	ss << "]";
	return ss.str();
}
bool token::isValid() const
{
	return !empty_;
}
void token::idToken()
{
	if (!context_.isSafe())
	{
		context_.reportError("Errors happened earlier, as a result tokens won't be identified.");
		return;
	}
	if (str.size() == 0)
		throw Exceptions::lexer_critical_error("Found an empty token. [" + pos.asText() + "]");

	// substract the token length's fron the column number given by the lexer.
	pos.column -= (int)str.length();

	if (specific_idSign())
		type = tokenType::TT_SIGN;
	else
	{
		if (specific_idKeyword())
			type = tokenType::TT_KEYWORD;
		else if (specific_idValue())
			type = tokenType::TT_LITERAL;
		else if (std::regex_match(str, kId_regex))
			type = tokenType::TT_IDENTIFIER;
		else
			context_.reportError("Could not identify a token -> (str) : " + str + "\t[" + pos.asText() + "]");
	}
}

bool token::specific_idKeyword()
{
	auto i = kWords_dict.find(str);
	if (i == kWords_dict.end())
		return false;
	
	kw_type = i->second;
	return true;
}

bool token::specific_idSign()
{
	if (str.size() > 1)
		return false;
	if (isdigit(str[0]))
		return false;
	auto i = kSign_dict.find(str[0]);
	if (i != kSign_dict.end())
	{
		sign_type = i->second;
		return true;
	}
	return false;
}

bool token::specific_idValue()
{
	std::stringstream converter(str);
	if (str[0] == '\'' )
	{
		if (str.back() == '\'')
		{
			str = str[1];
			if (str == "\\" && str.size() == 4) // If we have a \n in a char or something
				str += str[2];
			val_type = literalType::LIT_CHAR;
			return true;
		}
		else
		{
			context_.reportError("Unclosed char " + str);
			return false;
		}
	}
	else if (str[0] == '"')
	{
		if (str.back() == '"')
		{
			str = str.substr(1, str.size() - 2);
			val_type = literalType::LIT_STRING;
			return true;
		}
		else
		{
			context_.reportError("Unclosed string: " + str);
			return false;
		}
	}
	else if (str == "true" | str == "false")
	{
		vals = (str == "true" ? true : false);
		val_type = literalType::LIT_BOOL;
		return true;
	}
	// Might rework this bit later because it's a bit ugly, but it works !
	else if (std::regex_match(str, kInt_regex))
	{
		try
		{
			vals = std::stoi(str);
			val_type = literalType::LIT_INTEGER;
		}
		catch (std::out_of_range&)
		{
			// If out of range, try to put the value in a float instead.
			vals = std::stof(str);
			val_type = literalType::LIT_FLOAT;
		}
		return true;
	}
	else if (std::regex_match(str, kFloat_regex))
	{
		vals = std::stof(str);
		val_type = literalType::LIT_FLOAT;
		return true;
	}
	return false;
}

text_pos::text_pos()
{
}

text_pos::text_pos(const int & l, const int & col) : line(l), column(col)
{

}

void text_pos::newLine()
{
	line ++;
	//	column = 0;
}

void text_pos::forward()
{
//	column += 1;
}

std::string text_pos::asText() const
{
	std::stringstream ss;
	ss << "LINE:" << line /*<< " C:" << column*/;
	return ss.str();
}
