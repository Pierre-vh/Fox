////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Token.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Token.hpp"
#include "StringManipulator.hpp"

#include <regex>	// std::regex, std::regex_match
#include <string>	// std::stoi / stoll
#include <sstream>	// std::stringstream (showFormattedToken())
#include <cassert>

#include "Moonshot/Common/Context/Context.hpp"
#include "Moonshot/Common/Utils/Utils.hpp"
#include "Moonshot/Common/Exceptions/Exceptions.hpp"

using namespace Moonshot;
using namespace Moonshot::Dictionaries;

// Regular expression used for identification 
std::regex kInt_regex("\\d+");
std::regex kFloat_regex("[0-9]*\\.?[0-9]+");
std::regex kId_regex("(([A-Z]|[a-z]|_)([A-Z]|[0-9]|[a-z]|_)?)+");

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

LiteralType LiteralInfo::getType() const
{
	if (std::holds_alternative<bool>(val_))
		return LiteralType::Ty_Bool;
	else if (std::holds_alternative<std::string>(val_))
		return LiteralType::Ty_String;
	else if (std::holds_alternative<float>(val_))
		return LiteralType::Ty_Float;
	else if (std::holds_alternative<int64_t>(val_))
		return LiteralType::Ty_Int;
	else if (std::holds_alternative<char32_t>(val_))
		return LiteralType::Ty_Char;
	return LiteralType::DEFAULT;
}

LiteralInfo::LiteralInfo(const bool & bval)
{
	val_ = bval;
}

LiteralInfo::LiteralInfo(const std::string & sval)
{
	val_ = sval;
}

LiteralInfo::LiteralInfo(const float& fval)
{
	val_ = fval;
}

LiteralInfo::LiteralInfo(const int64_t& ival)
{
	val_ = ival;
}

LiteralInfo::LiteralInfo(const char32_t& cval)
{
	val_ = cval;
}

bool LiteralInfo::isNull() const
{
	return std::holds_alternative<std::monostate>(val_);
}

LiteralInfo::operator bool() const
{
	return !isNull();
}

Token::Token(Context *ctxt, std::string data, const TextPosition & tpos)
{
	// Check if ctxt isn't null
	assert(ctxt && "Context ptr is null!");
	context_ = ctxt;
	str_ = data;
	position_ = tpos;

	idToken();
}

Token::Token(const Token & cpy)
{
	context_ = cpy.context_;
	str_ = cpy.str_;
	position_ = cpy.position_;
	tokenInfo_ = cpy.tokenInfo_;


	if (cpy.litInfo_)
		litInfo_ = std::make_unique<LiteralInfo>(*(cpy.litInfo_));
	else
		litInfo_ = nullptr;
}

std::string Token::showFormattedTokenData() const
{
	if (!isValid())
		return "<INVALID TOKEN>"; // return nothing.

	std::stringstream ss;
	ss << "[Token][String: \"" << str_ << "\"][Position: " << position_.asText() << "][Type: " << getTokenTypeFriendlyName();
	int enumInfo = -1;

	if (isKeyword())
		enumInfo = Util::enumAsInt(std::get<KeywordType>(tokenInfo_));
	else if (isLiteral())
	{
		assert(litInfo_ && "Token is a literal but does not have a literalInfo?");
		enumInfo = Util::enumAsInt(litInfo_->getType());
	}
	else if (isSign())
		enumInfo = Util::enumAsInt(std::get<SignType>(tokenInfo_));

	if (enumInfo >= 0)
		ss << " (" << enumInfo << ")";

	ss << "]";

	return ss.str();
}

bool Token::isValid() const
{
	return !(std::holds_alternative<std::monostate>(tokenInfo_));
}

Token::operator bool() const
{
	return isValid();
}

bool Token::isLiteral() const
{
	return std::holds_alternative<Literal>(tokenInfo_);
}

bool Token::isIdentifier() const
{
	return std::holds_alternative<Identifier>(tokenInfo_);
}

bool Token::isSign() const
{
	return std::holds_alternative<SignType>(tokenInfo_);
}

bool Token::isKeyword() const
{
	return std::holds_alternative<KeywordType>(tokenInfo_);
}

KeywordType Token::getKeywordType() const
{
	if (isKeyword())
		return std::get<KeywordType>(tokenInfo_);
	return KeywordType::DEFAULT;
}

SignType Token::getSignType() const
{
	if (isSign())
		return std::get<SignType>(tokenInfo_);
	return SignType::DEFAULT;
}

std::string Token::getString() const
{
	return str_;
}

LiteralType Token::getLiteralType() const
{
	if (isLiteral())
	{
		assert(litInfo_ && "Token is a literal but does not have a literalInfo?");
		return litInfo_->getType();
	}
	return LiteralType::DEFAULT;
}

LiteralInfo Token::getLiteralInfo() const
{
	if (isLiteral())
	{
		assert(litInfo_ && "Token is a literal but does not have a literalInfo?");
		return *litInfo_;
	}
	return LiteralInfo();
}

void Token::idToken()
{
	if (str_.size() == 0)
		throw Exceptions::lexer_critical_error("Found an empty Token. [" + position_.asText() + "]");

	// substract the Token length's fron the column number given by the lexer.
	position_.column -= static_cast<unsigned int>(str_.length());

	if (specific_idSign());
	else if (specific_idKeyword());
	else if (specific_idLiteral());
	else if (std::regex_match(str_, kId_regex))
		tokenInfo_ = Identifier();
}

bool Token::specific_idKeyword()
{
	auto i = kKeywords_dict.find(str_);
	if (i == kKeywords_dict.end())
		return false;

	tokenInfo_ = i->second;
	return true;
}

bool Token::specific_idSign()
{
	if ((str_.size() > 1) || isdigit(str_[0]))
		return false;

	auto i = kSign_dict.find(str_[0]);
	if (i == kSign_dict.end())
		return false;

	tokenInfo_ = i->second;
	return true;
}


bool Token::specific_idLiteral()
{
	assert(context_ && "Cannot attempt to identify a literal without a context.");

	UTF8::StringManipulator strmanip;
	strmanip.setStr(str_);
	if (strmanip.peekFirst() == '\'')
	{
		if (strmanip.peekBack() == '\'')
		{
			if (strmanip.getSize() > 3)
			{
				context_->reportError("Char literal can only contain one character.");
				return false;
			}
			auto charlit = strmanip.getChar(1);
			tokenInfo_ = Literal();
			litInfo_ = std::make_unique<LiteralInfo>(charlit);
			return true;
		}
		else
		{
			context_->reportError("Char literal was not correctly closed.");
			return false;
		}
	}
	else if (strmanip.peekFirst() == '"')
	{
		if (strmanip.peekBack() == '"')
		{

			std::string strlit = strmanip.substring(1, strmanip.getSize() - 2); // Get the str between " ". Since "" are both 1 byte ascii char we don't need to use the strmanip.
			tokenInfo_ = Literal();
			litInfo_ = std::make_unique<LiteralInfo>(strlit);
			return true;
		}
		else
		{
			context_->reportError("String literal was not correctly closed.");
			return false;
		}
	}
	else if (str_ == "true" | str_ == "false")
	{
		tokenInfo_ = Literal();
		litInfo_ = std::make_unique<LiteralInfo>((str_ == "true" ? true : false));
		return true;
	}
	// Might rework this bit later because it's a bit ugly, but it works !
	else if (std::regex_match(str_, kInt_regex))
	{
		std::istringstream ss(str_);
		int64_t tmp;
		if (ss >> tmp)
		{
			tokenInfo_ = Literal();
			litInfo_ = std::make_unique<LiteralInfo>(tmp);
		}
		else
		{
			// If out of range, try to put the value in a float instead.
			std::stringstream out;
			out << "The value \xAF" << str_ << "\xAE was interpreted as a float because it didn't fit a 64 Bit signed int.";
			context_->reportWarning(out.str());
			tokenInfo_ = Literal();
			litInfo_ = std::make_unique<LiteralInfo>(std::stof(str_));
		}
		return true;
	}
	else if (std::regex_match(str_, kFloat_regex))
	{
		tokenInfo_ = Literal();
		litInfo_ = std::make_unique<LiteralInfo>(std::stof(str_));
		return true;
	}
	return false;
}

std::string Token::getTokenTypeFriendlyName() const
{
	if (isIdentifier())
		return "Identifier";
	else if (isKeyword())
		return "Keyword";
	else if (isLiteral())
		return "Literal";
	else if (isSign())
		return "Sign";
	return "Unknown Token Type";
}

TextPosition Token::getPosition() const
{
	return position_;
}