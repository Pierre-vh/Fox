//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : Token.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Lexer/Token.hpp"
#include <regex>
#include <string>
#include <sstream>
#include "Fox/Common/StringManipulator.hpp"
#include "Fox/Common/Source.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/AST/Identifiers.hpp"
#include "Fox/AST/ASTContext.hpp"

using namespace fox;
using namespace fox::dicts;

// Regular expression used for identification 
std::regex kInt_regex("\\d+");
std::regex kFloat_regex("[0-9]*\\.?[0-9]+");

template<typename T>
auto enumAsInt(T val) {
  return static_cast<typename std::underlying_type<T>::type>(val);
}

LiteralType LiteralInfo::getType() const {
  if (mpark::holds_alternative<bool>(value_))
    return LiteralType::Ty_Bool;
  else if (mpark::holds_alternative<std::string>(value_))
    return LiteralType::Ty_String;
  else if (mpark::holds_alternative<FoxFloat>(value_))
    return LiteralType::Ty_Float;
  else if (mpark::holds_alternative<FoxInt>(value_))
    return LiteralType::Ty_Int;
  else if (mpark::holds_alternative<FoxChar>(value_))
    return LiteralType::Ty_Char;
  return LiteralType::DEFAULT;
}

bool LiteralInfo::isBool() const {
  return mpark::holds_alternative<bool>(value_);
}

bool LiteralInfo::isString() const {
  return mpark::holds_alternative<std::string>(value_);
}

bool LiteralInfo::isFloat() const {
  return mpark::holds_alternative<FoxFloat>(value_);
}

bool LiteralInfo::isInt() const {
  return mpark::holds_alternative<FoxInt>(value_);
}

bool LiteralInfo::isChar() const {
  return mpark::holds_alternative<FoxChar>(value_);
}

FoxString LiteralInfo::getAsString() const {
  if (isBool())
    return get<bool>() ? "true" : "false";
  if (isString())
    return get<std::string>();
  if (isFloat())
    return std::to_string(get<FoxFloat>());
  if (isInt())
    return std::to_string(get<FoxInt>());
  if (isChar()) {
    std::string tmp;
    StringManipulator::append(tmp, get<FoxChar>());
    return tmp;
  }
  return "";
}

LiteralInfo::LiteralInfo(FoxBool bval) {
  value_ = bval;
}

LiteralInfo::LiteralInfo(const FoxString& sval) {
  value_ = sval;
}

LiteralInfo::LiteralInfo(FoxFloat fval) {
  value_ = fval;
}

LiteralInfo::LiteralInfo(FoxInt ival) {
  value_ = ival;
}

LiteralInfo::LiteralInfo(FoxChar cval) {
  value_ = cval;
}

bool LiteralInfo::isNull() const {
  return mpark::holds_alternative<mpark::monostate>(value_);
}

Token::Token(DiagnosticEngine &diags, ASTContext &astctxt,
  const std::string& tokstr, SourceRange range):
  range_(range) {
  identify(diags,astctxt,tokstr);
}

Token::Token(const Token& cpy):
  range_(cpy.range_), tokenData_(cpy.tokenData_) {
  if (cpy.literalData_)
    literalData_ = std::make_unique<LiteralInfo>(*(cpy.literalData_));
  else
    literalData_ = nullptr;
}

std::string Token::showFormattedTokenData() const {
  if (!isValid())
    return "<INVALID TOKEN>"; // return nothing.

  std::stringstream ss;
  ss << "[Token][String: \"" << getAsString() << "\"][Type: " << getTokenTypeFriendlyName();
  int enumInfo = -1;

  if (isKeyword())
    enumInfo = enumAsInt(mpark::get<KeywordType>(tokenData_));
  else if (isLiteral()) {
    assert(literalData_ && "Token is a literal but does not have a literalInfo?");
    enumInfo = enumAsInt(literalData_->getType());
  }
  else if (isSign())
    enumInfo = enumAsInt(mpark::get<SignType>(tokenData_));

  if (enumInfo >= 0)
    ss << " (" << enumInfo << ")";

  ss << "]";

  return ss.str();
}

bool Token::isValid() const {
  return (!(mpark::holds_alternative<mpark::monostate>(tokenData_))) || literalData_;
}

Token::operator bool() const {
  return isValid();
}

bool Token::isLiteral() const {
  return literalData_ != nullptr;
}

bool Token::isIdentifier() const {
  return mpark::holds_alternative<Identifier*>(tokenData_);
}

bool Token::isSign() const {
  return mpark::holds_alternative<SignType>(tokenData_);
}

bool Token::isKeyword() const {
  return mpark::holds_alternative<KeywordType>(tokenData_);
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

bool Token::is(LiteralType ty) {
  if (isLiteral())
    return getLiteralType() == ty;
  return false;
}

KeywordType Token::getKeywordType() const {
  if (isKeyword())
    return mpark::get<KeywordType>(tokenData_);
  return KeywordType::DEFAULT;
}

SignType Token::getSignType() const {
  if (isSign())
    return mpark::get<SignType>(tokenData_);
  return SignType::DEFAULT;
}

std::string Token::getAsString() const {
  if (mpark::holds_alternative<KeywordType>(tokenData_)) {
    auto kwtype = mpark::get<KeywordType>(tokenData_);
    for (auto it = kKeywords_dict.begin(); it != kKeywords_dict.end(); it++) {
      if (it->second == kwtype)
        return it->first;
    }
    fox_unreachable("unknown keyword");
  }
  else if (mpark::holds_alternative<SignType>(tokenData_)) {
    auto signtype = mpark::get<SignType>(tokenData_);
    for (auto it = kSign_dict.begin(); it != kSign_dict.end(); it++) {
      if (it->second == signtype)
        return StringManipulator::charToStr(it->first);
    }
    fox_unreachable("unknown sign");
  }
  else if (literalData_)
    return literalData_->getAsString();
  else if (mpark::holds_alternative<Identifier*>(tokenData_)) {
    auto ptr = mpark::get<Identifier*>(tokenData_);
    assert(ptr && "Token's an identifier but the Identifier* is null?");
    return ptr->getStr();
  }
  else
    return "<empty>";
}

LiteralType Token::getLiteralType() const {
  if (isLiteral()) {
    assert(literalData_ && "Token is a literal but does not have a literalInfo?");
    return literalData_->getType();
  }
  return LiteralType::DEFAULT;
}

LiteralInfo Token::getLiteralInfo() const {
  assert(literalData_);
  return *literalData_;
}

std::string Token::getIdentifierString() const {
  if (mpark::holds_alternative<Identifier*>(tokenData_)) {
    auto ptr = mpark::get<Identifier*>(tokenData_);
    assert(ptr && "tokenInfo's a Identifier* but the pointer is null?");
    return ptr->getStr();
  }
  return "";
}

Identifier * Token::getIdentifierInfo() {
  if (mpark::holds_alternative<Identifier*>(tokenData_)) {
    auto ptr = mpark::get<Identifier*>(tokenData_);
    assert(ptr && "tokenInfo's a Identifier* but the pointer is null?");
    return ptr;
  }
  return nullptr;
}

void Token::identify(DiagnosticEngine& diags,ASTContext& astctxt,const std::string& str) {
  // If the token is empty, this means our lexer might be broken!
  assert(str.size() && "String cannot be empty!");

  if (idSign(str));
  else if (idKeyword(str));
  else if (idLiteral(diags, str));
  else if (idIdentifier(diags, astctxt, str));
  else
    diags.report(DiagID::lexer_cant_id_tok, range_).addArg(str);
}

bool Token::idKeyword(const std::string& str) {
  auto i = kKeywords_dict.find(str);
  if (i == kKeywords_dict.end())
    return false;

  tokenData_ = i->second;
  return true;
}

bool Token::idSign(const std::string& str) {
  if ((str.size() > 1) || isdigit(str[0]))
    return false;

  auto i = kSign_dict.find(str[0]);
  if (i == kSign_dict.end())
    return false;

  tokenData_ = i->second;
  return true;
}

bool Token::idLiteral(DiagnosticEngine& diags, const std::string& str) {
  StringManipulator strmanip(str);
  if (strmanip.peekFirst() == '\'') {
    if (strmanip.peekBack() == '\'') {
      if (strmanip.getSizeInCodepoints() > 3) {
        diags.report(DiagID::lexer_too_many_char_in_char_literal, range_).addArg(str);
        return false;
      }
      else if (strmanip.getSizeInCodepoints() < 3) {
        // Empty char literal error is already handled by the lexer itself
        return false;
      }
      auto charlit = strmanip.getChar(1);
      tokenData_ = mpark::monostate();
      literalData_ = std::make_unique<LiteralInfo>(charlit);
      return true;
    }
    return false;
  }
  else if (strmanip.peekFirst() == '"') {
    if (strmanip.peekBack() == '\"') {
      std::string strlit = strmanip.substring(1, strmanip.getSizeInCodepoints() - 2); // Get the str between " ". Since "" are both 1 byte ascii char we don't need to use the strmanip.
      tokenData_ = mpark::monostate();
      literalData_ = std::make_unique<LiteralInfo>(strlit);
      return true;
    }
    return false;
  }
  else if (str == "true" | str == "false") {
    tokenData_ = mpark::monostate();
    literalData_ = std::make_unique<LiteralInfo>((str == "true" ? true : false));
    return true;
  }
  // Might rework this bit later because it's a bit ugly, but it works !
  else if (std::regex_match(str, kInt_regex)) {
    std::istringstream ss(str);
    FoxInt tmp;
    if (ss >> tmp) {
      tokenData_ = mpark::monostate();
      literalData_ = std::make_unique<LiteralInfo>(tmp);
    }
    else {
      // If too big, put the value in a float instead.
      diags.report(DiagID::lexer_int_too_big_considered_as_float, range_).addArg(str);
      tokenData_ = mpark::monostate();
      literalData_ = std::make_unique<LiteralInfo>(std::stof(str));
    }
    return true;
  }
  else if (std::regex_match(str, kFloat_regex)) {
    tokenData_ = mpark::monostate();
    literalData_ = std::make_unique<LiteralInfo>(std::stof(str));
    return true;
  }
  return false;
}

bool Token::idIdentifier(DiagnosticEngine& diags,ASTContext& astctxt, const std::string & str) {
  if (validateIdentifier(diags,str)) {
    tokenData_ = astctxt.identifiers.getUniqueIdentifierInfo(str);
    return true;
  }
  return false;
}

bool Token::validateIdentifier(DiagnosticEngine& diags, const std::string& str) const {
  // Identifiers : An Identifier's first letter must always be a underscore or an alphabetic letter
  // The first character can then be followed by an underscore, a letter or a number.
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

SourceRange Token::getRange() const {
  return range_;
}

bool Token::hasAtLeastOneLetter(const std::string& str) const {
  return std::any_of(str.begin(), str.end(), ::isalpha);
}
