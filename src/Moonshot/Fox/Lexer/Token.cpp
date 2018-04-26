////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Token.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Token.hpp"


#include <regex>
#include <string>
#include <sstream>
#include <cassert>

#include "StringManipulator.hpp"
#include "Moonshot/Fox/Basic/Context.hpp"
#include "Moonshot/Fox/Basic/Utils.hpp"
#include "Moonshot/Fox/Basic/Exceptions.hpp"
#include "Moonshot/Fox/AST/IdentifierTable.hpp"
#include "Moonshot/Fox/AST/ASTContext.hpp"

using namespace Moonshot;
using namespace Moonshot::Dictionaries;

// Regular expression used for identification 
std::regex kInt_regex("\\d+");
std::regex kFloat_regex("[0-9]*\\.?[0-9]+");



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
	else if (std::holds_alternative<FloatType>(val_))
		return LiteralType::Ty_Float;
	else if (std::holds_alternative<IntType>(val_))
		return LiteralType::Ty_Int;
	else if (std::holds_alternative<CharType>(val_))
		return LiteralType::Ty_Char;
	return LiteralType::DEFAULT;
}

bool LiteralInfo::isBool() const
{
	return std::holds_alternative<bool>(val_);
}

bool LiteralInfo::isString() const
{
	return std::holds_alternative<std::string>(val_);
}

bool LiteralInfo::isFloat() const
{
	return std::holds_alternative<FloatType>(val_);
}

bool LiteralInfo::isInt() const
{
	return std::holds_alternative<IntType>(val_);
}

bool LiteralInfo::isChar() const
{
	return std::holds_alternative<CharType>(val_);
}

std::string LiteralInfo::getAsString() const
{
	if (isBool())
		return get<bool>() ? "true" : "false";
	if (isString())
		return get<std::string>();
	if (isFloat())
		return std::to_string(get<FloatType>());
	if (isInt())
		return std::to_string(get<IntType>());
	if (isChar())
	{
		std::string tmp;
		UTF8::StringManipulator::append(tmp, get<CharType>());
		return tmp;
	}
	return "";
}

LiteralInfo::LiteralInfo(const bool & bval)
{
	val_ = bval;
}

LiteralInfo::LiteralInfo(const std::string & sval)
{
	val_ = sval;
}

LiteralInfo::LiteralInfo(const FloatType& fval)
{
	val_ = fval;
}

LiteralInfo::LiteralInfo(const IntType& ival)
{
	val_ = ival;
}

LiteralInfo::LiteralInfo(const CharType& cval)
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

Token::Token(Context &ctxt, ASTContext &astctxt, std::string tokstr, const TextPosition & tpos)
{
	position_ = tpos;
	idToken(ctxt,astctxt,tokstr);
}

Token::Token(const Token & cpy)
{
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
	ss << "[Token][String: \"" << getAsString() << "\"][Position: " << position_.asText() << "][Type: " << getTokenTypeFriendlyName();
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
	return std::holds_alternative<IdentifierInfo*>(tokenInfo_);
}

bool Token::isSign() const
{
	return std::holds_alternative<SignType>(tokenInfo_);
}

bool Token::isKeyword() const
{
	return std::holds_alternative<KeywordType>(tokenInfo_);
}

bool Token::is(const KeywordType & ty)
{
	if (isKeyword())
		return getKeywordType() == ty;
	return false;
}

bool Token::is(const SignType & ty)
{
	if (isSign())
		return getSignType() == ty;
	return false;
}

bool Token::is(const LiteralType & ty)
{
	if (isLiteral())
		return getLiteralType() == ty;
	return false;
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

std::string Token::getAsString() const
{
	if (std::holds_alternative<KeywordType>(tokenInfo_))
	{
		auto kwtype = std::get<KeywordType>(tokenInfo_);
		for (auto it = kKeywords_dict.begin(); it != kKeywords_dict.end(); it++)
		{
			if (it->second == kwtype)
				return it->first;
		}
		throw std::exception("Unknown keyword type!");
	}
	else if (std::holds_alternative<SignType>(tokenInfo_))
	{
		auto signtype = std::get<SignType>(tokenInfo_);
		CharType ch = ' ';
		for (auto it = kSign_dict.begin(); it != kSign_dict.end(); it++)
		{
			if (it->second == signtype)
				ch = it->first;
		}
		std::string str = "";
		UTF8::StringManipulator::append(str, ch);
		return str;
	}
	else if (std::holds_alternative<Literal>(tokenInfo_))
	{
		assert(litInfo_ && "Token's a literal but no LiteralInfo available?");
		return litInfo_->getAsString();
	}
	else if (std::holds_alternative<IdentifierInfo*>(tokenInfo_))
	{
		auto ptr = std::get<IdentifierInfo*>(tokenInfo_);
		assert(ptr && "IdentifierInfo is null?");
		return ptr->getStr();
	}
	else
		return "<empty token>";
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

std::string Token::getIdentifierString() const
{
	if (std::holds_alternative<IdentifierInfo*>(tokenInfo_))
	{
		auto ptr = std::get<IdentifierInfo*>(tokenInfo_);
		assert(ptr && "tokenInfo's a IdentifierInfo* but the pointer is null?");
		return ptr->getStr();
	}
	return "";
}

IdentifierInfo * Token::getIdentifierInfo()
{
	if (std::holds_alternative<IdentifierInfo*>(tokenInfo_))
	{
		auto ptr = std::get<IdentifierInfo*>(tokenInfo_);
		assert(ptr && "tokenInfo's a IdentifierInfo* but the pointer is null?");
		return ptr;
	}
	return nullptr;
}

void Token::idToken(Context& ctxt,ASTContext& astctxt,const std::string& str)
{
	// If the token is empty, this means our lexer might be broken!
	assert(str.size() && "Token cannot be empty!");

	// substract the Token length's fron the column number given by the lexer.
	position_.column -= (unsigned)(str.length());

	if (specific_idSign(str));
	else if (specific_idKeyword(str));
	else if (specific_idLiteral(ctxt,str));
	else if (specific_idIdentifier(ctxt,astctxt,str));
	else
		ctxt.reportError("Could not identify token \"" + str + "\"");
}

bool Token::specific_idKeyword(const std::string& str)
{
	auto i = kKeywords_dict.find(str);
	if (i == kKeywords_dict.end())
		return false;

	tokenInfo_ = i->second;
	return true;
}

bool Token::specific_idSign(const std::string& str)
{
	if ((str.size() > 1) || isdigit(str[0]))
		return false;

	auto i = kSign_dict.find(str[0]);
	if (i == kSign_dict.end())
		return false;

	tokenInfo_ = i->second;
	return true;
}

bool Token::specific_idLiteral(Context& ctxt,const std::string& str)
{
	UTF8::StringManipulator strmanip;
	strmanip.setStr(str);
	if (strmanip.peekFirst() == '\'')
	{
		if (strmanip.peekBack() == '\'')
		{
			if (strmanip.getSize() > 3)
			{
				ctxt.reportError("Char literal can only contain one character.");
				return false;
			}
			else if (strmanip.getSize() < 3)
			{
				ctxt.reportError("Empty char literal");
				return false;
			}
			auto charlit = strmanip.getChar(1);
			tokenInfo_ = Literal();
			litInfo_ = std::make_unique<LiteralInfo>(charlit);
			return true;
		}
		else
		{
			ctxt.reportError("Char literal was not correctly closed.");
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
			ctxt.reportError("String literal was not correctly closed.");
			return false;
		}
	}
	else if (str == "true" | str == "false")
	{
		tokenInfo_ = Literal();
		litInfo_ = std::make_unique<LiteralInfo>((str == "true" ? true : false));
		return true;
	}
	// Might rework this bit later because it's a bit ugly, but it works !
	else if (std::regex_match(str, kInt_regex))
	{
		std::istringstream ss(str);
		IntType tmp;
		if (ss >> tmp)
		{
			tokenInfo_ = Literal();
			litInfo_ = std::make_unique<LiteralInfo>(tmp);
		}
		else
		{
			// If out of range, try to put the value in a float instead.
			std::stringstream out;
			out << "The value \"" << str << "\" was interpreted as a float because it didn't fit a 64 Bit signed int.";
			ctxt.reportWarning(out.str());
			tokenInfo_ = Literal();
			litInfo_ = std::make_unique<LiteralInfo>(std::stof(str));
		}
		return true;
	}
	else if (std::regex_match(str, kFloat_regex))
	{
		tokenInfo_ = Literal();
		litInfo_ = std::make_unique<LiteralInfo>(std::stof(str));
		return true;
	}
	return false;
}

bool Token::specific_idIdentifier(Context& ctxt,ASTContext& astctxt, const std::string & str)
{
	if (validateIdentifier(ctxt,str))
	{
		tokenInfo_ = astctxt.identifiers.getUniqueIdentifierInfo(str);
		return true;
	}
	return false;
}

bool Token::validateIdentifier(Context& ctxt,const std::string & str) const
{
	// Identifiers : An Identifier's first letter must always be a underscore or an alphabetic letter
	// The first character can then be followed by an underscore, a letter or a number.
	UTF8::StringManipulator manip(str);
	auto first_ch = manip.getCurrentChar();
	if ((first_ch == '_') || iswalpha((char)first_ch))
	{
		// First character is ok, proceed to identify the rest of the string
		for (manip.advance() /* skip first char*/; !manip.isAtEndOfStr(); manip.advance())
		{
			auto ch = manip.getCurrentChar();
			if ((ch != '_') && !iswalnum((char)ch))
			{
				ctxt.reportError("The identifier \"" + str + "\" contains invalid characters.");
				return false;
			}
		}
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

bool Token::hasAtLeastOneLetter(const std::string& str) const
{
	return std::any_of(str.begin(), str.end(), ::isalpha);
}