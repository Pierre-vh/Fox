//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
// See LICENSE.txt for license info.            
// File : Token.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Lexer/Token.hpp"
#include <regex>
#include <string>
#include <sstream>
#include "Fox/Common/StringManipulator.hpp"
#include "Fox/Common/SourceLoc.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/AST/ASTContext.hpp"

using namespace fox;
using namespace fox::dicts;

// Regular expression used for identification 
std::regex kInt_regex("\\d+");
std::regex kDouble_regex("[0-9]*\\.?[0-9]+");

template<typename T>
auto enumAsInt(T val) {
  return static_cast<typename std::underlying_type<T>::type>(val);
}

Token::Token(ASTContext& ctxt, const std::string& tokstr,
             SourceRange range):
  range_(range) {
  identify(ctxt, tokstr);
}

std::string Token::showFormattedTokenData() const {
  if (!isValid())
    return "<INVALID TOKEN>"; // return nothing.

  std::stringstream ss;
  ss << "[Token](" << getTokenTypeFriendlyName() << ") '" 
		<< getAsString() << "'";
  
  // For keywords and sign, show the precise enum value
  if (isKeyword())
    ss << " (KeywordType:" << enumAsInt(data_.keyword) << ")";
  else if (isSign())
    ss << " (SignType:" << enumAsInt(data_.keyword) << ")";

  return ss.str();
}

bool Token::isValid() const {
  return data_.kind != Kind::Invalid;
}

Token::operator bool() const {
  return isValid();
}

bool Token::isLiteral() const {
  switch (data_.kind) {
    case Kind::BoolLiteral:
    case Kind::CharLiteral:
    case Kind::DoubleLiteral:
    case Kind::IntLiteral:
    case Kind::StringLiteral:
      return true;
    default:
      return false;
  }
}

bool Token::isIdentifier() const {
  return data_.kind == Kind::Identifier;
}

bool Token::isSign() const {
  return data_.kind == Kind::Sign;
}

bool Token::isKeyword() const {
  return data_.kind == Kind::Keyword;
}

bool Token::isStringLiteral() const {
  return data_.kind == Kind::StringLiteral;
}

bool Token::isBoolLiteral() const {
  return data_.kind == Kind::BoolLiteral;
}

bool Token::isDoubleLiteral() const {
  return data_.kind == Kind::DoubleLiteral;
}

bool Token::isIntLiteral() const {
  return data_.kind == Kind::IntLiteral;
}

bool Token::isCharLiteral() const {
  return data_.kind == Kind::CharLiteral;
}

bool Token::is(KeywordType ty) {
  if (isKeyword())
    return getKeywordType() == ty;
  return false;
}

bool Token::is(SignType ty) {
  if (isSign())
    return getSignType() == ty;
  return false;
}

KeywordType Token::getKeywordType() const {
  assert(isKeyword() && "not a sign!");
  return data_.keyword;
}

SignType Token::getSignType() const {
  assert(isSign() && "not a sign!");
  return data_.sign;
}

Identifier Token::getIdentifier() const {
  assert(isIdentifier() && "not an identifier!");
  auto id = data_.identifier;
  assert(id && "Null Identifier object on a Identifier Token?");
  return id;
}

bool Token::getBoolValue() const {
  assert(isBoolLiteral() && "not a bool literal!");
  return data_.boolLiteral;
}

string_view Token::getStringValue() const {
  assert(isStringLiteral() && "not a string literal!");
  return data_.stringLiteral;
}

FoxChar Token::getCharValue() const {
  assert(isCharLiteral() && "not a char literal!");
  return data_.charLiteral;
}

FoxInt Token::getIntValue() const {
  assert(isIntLiteral() && "not an int literal!");
  return data_.intLiteral;
}

FoxDouble Token::getDoubleValue() const {
  assert(isDoubleLiteral() && "not a double literal!");
  return data_.doubleLiteral;
}

std::string Token::getAsString() const {
  std::stringstream ss;
  switch (data_.kind) {
    case Kind::BoolLiteral:
      ss << (data_.boolLiteral ? "true" : "false");
      break;
    case Kind::StringLiteral:
      ss << '"' << data_.stringLiteral << '"';
      break;
    case Kind::IntLiteral:
      ss << data_.intLiteral;
      break;
    case Kind::DoubleLiteral:
      ss << data_.doubleLiteral;
      break;
    case Kind::CharLiteral:
      ss << "'" << StringManipulator::charToStr(data_.charLiteral) << "'";
      break;
    case Kind::Keyword: {
      auto begin = kKeywords_dict.begin();
      auto end = kKeywords_dict.end();
      auto it = begin;
      bool found = false;
      while (it != end) {
        if (it->second == data_.keyword) {
          ss << it->first;
          found = true;
          break;
        }
        ++it;
      }
      assert(found && "Keyword not found in map!");
      break;
    }
    case Kind::Identifier:
      ss << data_.identifier.getStr();
      break;
    case Kind::Sign: {
      auto begin = kSign_dict.begin();
      auto end = kSign_dict.end();
      auto it = begin;
      bool found = false;
      while (it != end) {
        if (it->second == data_.sign) {
          ss << it->first;
          found = true;
          break;
        }
        ++it;
      }
      assert(found && "Sign not found in map!");
      break;
    }
    case Kind::Invalid:
      ss << "invalid";
      break;
    default:
      fox_unreachable("invalid kind");
  }
  return ss.str();
}

void 
Token::identify(ASTContext& ctxt, const std::string& str) {
  // If the token is empty, this means our lexer might be broken!
  assert(str.size() && "String cannot be empty!");

  if (idSign(str));
  else if (idKeyword(str));
  else if (idLiteral(ctxt, ctxt.diagEngine, str));
  else if (idIdentifier(ctxt, str));
  else
    ctxt.diagEngine.report(DiagID::lexer_cant_id_tok, range_).addArg(str);
}

bool Token::idKeyword(const std::string& str) {
  auto i = kKeywords_dict.find(str);
  if (i == kKeywords_dict.end())
    return false;

  data_.setKeyword(i->second);
  return true;
}

bool Token::idSign(const std::string& str) {
  if ((str.size() > 1) || isdigit(str[0]))
    return false;

  auto i = kSign_dict.find(str[0]);
  if (i == kSign_dict.end())
    return false;

  data_.setSign(i->second);
  return true;
}

bool Token::idLiteral(ASTContext& ctxt, DiagnosticEngine& diags, 
                      const std::string& str) {
  StringManipulator strmanip(str);
  if (strmanip.peekFirst() == '\'') {
    if (strmanip.peekBack() == '\'') {
      if (strmanip.getSizeInCodepoints() > 3) {
        diags
					.report(DiagID::lexer_too_many_char_in_char_literal, range_)
					.addArg(str);
        return false;
      }
      else if (strmanip.getSizeInCodepoints() < 3) {
        // Empty char literal error is already handled by the lexer itself
        return false;
      }
      auto charlit = strmanip.getChar(1);
      data_.setCharLiteral(charlit);
      return true;
    }
    return false;
  }
  else if (strmanip.peekFirst() == '"') {
    if (strmanip.peekBack() == '\"') {
			// Get the str between " ". Since "" are both 1 byte ascii 
			// char we don't need to use the strmanip.
      string_view strlit = strmanip.substring(1, 
				strmanip.getSizeInCodepoints() - 2);
      data_.setStringLiteral(ctxt.allocateCopy(strlit));
      return true;
    }
    return false;
  }
  else if (str == "true" || str == "false") {
    data_.setBoolLiteral(str == "true");
    return true;
  }
  else if (std::regex_match(str, kInt_regex)) {
    std::istringstream ss(str);
    FoxInt tmp;
    if (ss >> tmp)
      data_.setIntLiteral(tmp);
    else {
      // If too big, put the value in a double instead.
      diags
				.report(DiagID::lexer_int_too_big_considered_as_double, range_)
				.addArg(str);
      data_.setDoubleLiteral(std::stod(str));
    }
    return true;
  }
  else if (std::regex_match(str, kDouble_regex)) {
    data_.setDoubleLiteral(std::stod(str));
    return true;
  }
  return false;
}

bool Token::idIdentifier(ASTContext& ctxt, const std::string& str) {
  if (validateIdentifier(ctxt.diagEngine, str)) {
    data_.setIdentifier(ctxt.getIdentifier(str));
    return true;
  }
  return false;
}

bool Token::validateIdentifier(DiagnosticEngine& diags, 
	const std::string& str) const {
  // Identifiers : An Identifier's first letter must 
	// always be a underscore or an alphabetic letter
  // The first character can then be followed 
	// by an underscore, a letter or a number.
  StringManipulator manip(str);
  auto first_ch = manip.getCurrentChar();
  if ((first_ch == '_') || iswalpha((char)first_ch)) {
    // First character is ok, proceed to identify the rest of the string
    for (manip.advance() /* skip first char*/; !manip.eof(); manip.advance()) {
      auto ch = manip.getCurrentChar();
      if ((ch != '_') && !iswalnum((char)ch)) {
        diags.report(DiagID::lexer_invalid_char_in_id, range_).addArg(str);
        return false;
      }
    }
    return true;
  }
  return false;
}

std::string Token::getTokenTypeFriendlyName() const {
  switch (data_.kind) {
    case Kind::BoolLiteral:
      return "BoolLiteral";
    case Kind::StringLiteral:
      return "BoolLiteral";
    case Kind::IntLiteral:
      return "IntLiteral";
    case Kind::DoubleLiteral:
      return "DoubleLiteral";
    case Kind::CharLiteral:
      return "CharLiteral";
    case Kind::Keyword:
      return "Keyword";
    case Kind::Identifier:
      return "Identifier";
    case Kind::Sign:
      return "Sign";
    case Kind::Invalid:
      return "Invalid";
    default:
      fox_unreachable("invalid kind");
  }
}

SourceRange Token::getRange() const {
  return range_;
}

bool Token::hasAtLeastOneLetter(const std::string& str) const {
  return std::any_of(str.begin(), str.end(), ::isalpha);
}
