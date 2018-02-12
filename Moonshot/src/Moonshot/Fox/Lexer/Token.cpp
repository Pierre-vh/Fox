////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Token.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Token.hpp"

#include <variant>	// std::variant
#include <regex>	// std::regex, std::regex_match
#include <string>	// std::stoi / stoll
#include <sstream>	// std::stringstream (showFormattedToken())

#include "Moonshot/Common/Utils/Utils.hpp"
#include "Moonshot/Common/Context/Context.hpp"
#include "Moonshot/Common/Exceptions/Exceptions.hpp"
#include "Moonshot/Common/UTF8/StringManipulator.hpp"

using namespace Moonshot;

// Regular expression used for identification 
std::regex kInt_regex("\\d+");
std::regex kFloat_regex("[0-9]*\\.?[0-9]+");
std::regex kId_regex("(([A-Z]|[a-z]|_)([A-Z]|[0-9]|[a-z]|_)?)+");

Token::Token(Context & c) : context_(c)
{
}
Token::Token(Context & c,std::string data, const TextPosition &tpos) : context_(c),str(data),pos(tpos)
{
	idToken(); // self id
}

std::string Token::showFormattedTokenData() const
{
	if (str.size() == 0) // Token is empty
		return "<EMPTY TOKEN>"; // return nothing.

	std::stringstream ss;
	ss << "[str:\"" << str << "\"][" << pos.asText() << "][type:";
	int enum_info = -1;		// The information of the corresponding enumeration
	switch (type)
	{
		case tokenCat::TT_ENUM_DEFAULT:
			ss << "ENUM_DEFAULT";
			break;
		case tokenCat::TT_IDENTIFIER:
			ss << "IDENTIFIER";
			enum_info = -2;
			break;
		case tokenCat::TT_KEYWORD:
			ss << "KEYWORD";
			enum_info = util::enumAsInt(kw_type);
			break;
		case tokenCat::TT_SIGN:
			ss << "SIGN";
			enum_info = util::enumAsInt(sign_type);
			break;
		case tokenCat::TT_LITERAL:
			ss << "VALUE";
			enum_info = util::enumAsInt(lit_type);
			break;
	}
	if (enum_info >= -1)
		ss << " -> E:" << enum_info;
	ss << "]";
	return ss.str();
}
bool Token::isValid() const
{
	return str.size();
}
void Token::idToken()
{
	/*
	if (!context_.isSafe())
	{
		context_.reportError("Errors happened earlier, as a result tokens won't be identified.");
		return;
	}
	*/
	if (str.size() == 0)
		throw Exceptions::lexer_critical_error("Found an empty Token. [" + pos.asText() + "]");

	// substract the Token length's fron the column number given by the lexer.
	pos.column -= (int)str.length();

	if (specific_idSign())
		type = tokenCat::TT_SIGN;
	else
	{
		if (specific_idKeyword())
			type = tokenCat::TT_KEYWORD;
		else if (specific_idLiteral())
			type = tokenCat::TT_LITERAL;
		else if (std::regex_match(str, kId_regex))
			type = tokenCat::TT_IDENTIFIER;
		else
			context_.reportError(" [" + pos.asText() + "]\tUnrecognized Token \"" + str + '"');
	}
}

bool Token::specific_idKeyword()
{
	auto i = kWords_dict.find(str);
	if (i == kWords_dict.end())
		return false;
	
	kw_type = i->second;
	return true;
}

bool Token::specific_idSign()
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

bool Token::specific_idLiteral()
{
	std::stringstream converter(str);
	UTF8::StringManipulator strmanip;
	strmanip.setStr(str);

	if (strmanip.peekFirst() == '\'')
	{
		if (strmanip.peekBack() == '\'')
		{
			if (strmanip.getSize() > 3)
			{
				context_.reportError("Char literal can only contain one character.");
				return false;
			}
			lit_val = strmanip.getChar(1);
			lit_type = literal::LIT_CHAR;
			return true;
		}
		else
		{
			context_.reportError("Char literal was not correctly closed.");
			return false;
		}
	}
	else if (strmanip.peekFirst() == '"')
	{
		if (strmanip.peekBack() == '"')
		{
			lit_val = strmanip.substring(1,strmanip.getSize()-2); // Get the str between " ". Since "" are both 1 byte ascii char we don't need to use the strmanip.
			lit_type = literal::LIT_STRING;
			return true;
		}
		else
		{
			context_.reportError("String literal was not correctly closed.");
			return false;
		}
	}
	else if (str == "true" | str == "false")
	{
		lit_val = (str == "true" ? true : false);
		lit_type = literal::LIT_BOOL;
		return true;
	}
	// Might rework this bit later because it's a bit ugly, but it works !
	else if (std::regex_match(str, kInt_regex))
	{
		std::istringstream ss(str);
		int64_t tmp;
		if(ss >> tmp)
		{
			lit_val = tmp;
			lit_type = literal::LIT_INTEGER;
		}
		else
		{
			// If out of range, try to put the value in a float instead.
			std::stringstream out;
			out << "The value \xAF" << str << "\xAE was interpreted as a float because it didn't fit a 64 Bit signed int.";
			context_.reportWarning(out.str());
			lit_val = std::stof(str);
			lit_type = literal::LIT_FLOAT;
		}
		return true;
	}
	else if (std::regex_match(str, kFloat_regex))
	{
		lit_val = std::stof(str);
		lit_type = literal::LIT_FLOAT;
		return true;
	}
	return false;
}

TextPosition::TextPosition()
{
}

TextPosition::TextPosition(const int & l, const int & col) : line(l), column(col)
{

}

void TextPosition::newLine()
{
	line ++;
	//	column = 0;
}

void TextPosition::forward()
{
//	column += 1;
}

std::string TextPosition::asText() const
{
	std::stringstream ss;
	ss << "LINE:" << line /*<< " C:" << column*/;
	return ss.str();
}
