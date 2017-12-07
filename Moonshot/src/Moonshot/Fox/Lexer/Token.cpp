#include "Token.h"
#include "../../Common/Errors/Errors.h"
using namespace Moonshot;

token::token()
{
	empty_ = true;
}
token::token(std::string data, const text_pos &tpos) : str(data),pos(tpos)
{
	// substract the token length's fron the column number given by the lexer.
	pos.column -= (int)data.length();
	// self id
	selfId();
}

std::string token::showFormattedTokenData() const
{
	std::stringstream ss;
	ss << "[str:\"" << str << "\"][pos:" << pos.asText() << "][type:";
	int enum_info = -1;		// The information of the corresponding enumeration
	switch (type)
	{
		case lex::TT_ENUM_DEFAULT:
			ss << "ENUM_DEFAULT";
			break;
		case lex::TT_IDENTIFIER:
			ss << "IDENTIFIER";
			enum_info = -2;
			break;
		case lex::TT_KEYWORD:
			ss << "KEYWORD";
			enum_info = kw_type;
			break;
		case lex::TT_SIGN:
			ss << "SIGN";
			enum_info = sign_type;
			break;
		case lex::TT_VALUE:
			ss << "VALUE";
			enum_info = val_type;
			break;
	}
	if (enum_info >= -1)
		ss << " -> E:" << enum_info;
	ss << "]";
	return ss.str();
}
bool Moonshot::token::isValid() const
{
	return !empty_;
}
void token::selfId()
{
	if (str.size() == 0)
		E_CRITICAL("Found an empty token. [" + pos.asText() + "]")

	if (idSign())
		type = lex::TT_SIGN;
	else
	{
		if (idKeyword())
			type = lex::TT_KEYWORD;
		else if (idValue())
			type = lex::TT_VALUE;
		else if (std::regex_match(str, lex::kId_regex))
			type = lex::TT_IDENTIFIER;
		else 
			E_ERROR("Could not identify a token (str) : " + str + "\t[" + pos.asText() + "]")
	}
}

bool token::idKeyword()
{
	auto i = lex::kWords_dict.find(str);
	if (i == lex::kWords_dict.end())
		return false;
	
	kw_type = i->second;
	return true;
}

bool token::idSign()
{
	if (str.size() > 1)
		return false;
	if (isdigit(str[0]))
		return false;
	auto i = lex::kSign_dict.find(str[0]);
	if (i == lex::kSign_dict.end())
		return true;

	sign_type = i->second;
	return true;
}

bool token::idValue()
{
	if (str[0] == '\'' )
	{
		if (str.back() == '\'')
		{
			str = str[1];
			if (str == "\\" && str.size() == 4) // If we have a \n in a char or something
				str += str[2];
			val_type = lex::VAL_CHAR;
			return true;
		}
		else
		{
			E_ERROR("Unclosed char " + str);
			return false;
		}
	}
	else if (str[0] == '"')
	{
		if (str.back() == '"')
		{
			str = str.substr(1, str.size() - 2);
			val_type = lex::VAL_STRING;
			return true;
		}
		else
		{
			E_ERROR("Unclosed string: " + str);
			return false;
		}
	}
	else if (str == "true" | str == "false")
	{
		vals = (str == "true" ? true : false);
		val_type = lex::VAL_BOOL;
		return true;
	}
	else if (std::regex_match(str, lex::kInt_regex))
	{
		vals = std::stoi(str);
		val_type = lex::VAL_INTEGER;
		return true;
	}
	else if (std::regex_match(str, lex::kFloat_regex))
	{
		vals = std::stof(str);
		val_type = lex::VAL_FLOAT;
		return true;
	}
	return false;
}

Moonshot::text_pos::text_pos()
{
}

text_pos::text_pos(const int & l, const int & col) : line(l), column(col)
{

}

void text_pos::newLine()
{
	line += 1;
	column = 0;
}

void text_pos::forward()
{
	column += 1;
}

std::string text_pos::asText() const
{
	std::stringstream ss;
	ss << "L:" << line << " C:" << column;
	return ss.str();
}
