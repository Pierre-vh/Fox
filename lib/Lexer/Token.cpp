//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : Token.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Lexer/Token.hpp"
#include "Fox/Common/Errors.hpp"

using namespace fox;

template<typename T>
auto enumAsInt(T val) {
  return static_cast<typename std::underlying_type<T>::type>(val);
}

Token::Token(Kind kind, string_view str, SourceRange range) :
  kind(kind), str(str), range(range) {
  assert(range && "Token constructed with invalid an SourceRange");
}

Token::Token(SignType sign, string_view str, SourceRange range) 
  : Token(Kind::Sign, str, range) {
  signType_ = sign;
}

Token::Token(KeywordType kw, string_view str, SourceRange range) 
  : Token(Kind::Keyword, str, range) {
  kwType_ = kw;
}

bool Token::isValid() const {
  return kind != Kind::Invalid;
}

Token::operator bool() const {
  return isValid();
}

bool Token::isLiteral() const {
  switch (kind) {
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
  return kind == Kind::Identifier;
}

bool Token::isSign() const {
  return kind == Kind::Sign;
}

bool Token::isKeyword() const {
  return kind == Kind::Keyword;
}

bool Token::isStringLiteral() const {
  return kind == Kind::StringLiteral;
}

bool Token::isBoolLiteral() const {
  return kind == Kind::BoolLiteral;
}

bool Token::isDoubleLiteral() const {
  return kind == Kind::DoubleLiteral;
}

bool Token::isIntLiteral() const {
  return kind == Kind::IntLiteral;
}

bool Token::isCharLiteral() const {
  return kind == Kind::CharLiteral;
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
  return kwType_;
}

SignType Token::getSignType() const {
  assert(isSign() && "not a sign!");
  return signType_;
}

void Token::dump(std::ostream& out) const {
  out << str;
}
