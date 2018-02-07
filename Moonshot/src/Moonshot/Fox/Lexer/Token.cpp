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


Token::Token(Context & c) : context_(c)
{
	empty_ = true;
}
Token::Token(Context & c,std::string data, const text_pos &tpos) : context_(c),str(data),pos(tpos)
{
	idToken(); // self id
}

std::string Token::showFormattedTokenData() const
{
	if (empty_) // Token is empty
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
	return !empty_;
}
void Token::idToken()
{
	if (!context_.isSafe())
	{
		context_.reportError("Errors happened earlier, as a result tokens won't be identified.");
		return;
	}
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
		else if (specific_idValue())
			type = tokenCat::TT_LITERAL;
		else if (std::regex_match(str, kId_regex))
			type = tokenCat::TT_IDENTIFIER;
		else
			context_.reportError("Could not identify a Token -> (str) : " + str + "\t[" + pos.asText() + "]");
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

bool Token::specific_idValue()
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
			vals = strmanip.getChar(1);
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
			vals = strmanip.substring(1,strmanip.getSize()-2); // Get the str between " ". Since "" are both 1 byte ascii char we don't need to use the strmanip.
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
		vals = (str == "true" ? true : false);
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
			vals = tmp;
			lit_type = literal::LIT_INTEGER;
		}
		else
		{
			// If out of range, try to put the value in a float instead.
			std::stringstream out;
			out << "The value \xAF" << str << "\xAE was interpreted as a float because it didn't fit a 64 Bit signed int.";
			context_.reportWarning(out.str());
			vals = std::stof(str);
			lit_type = literal::LIT_FLOAT;
		}
		return true;
	}
	else if (std::regex_match(str, kFloat_regex))
	{
		vals = std::stof(str);
		lit_type = literal::LIT_FLOAT;
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
