//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : Token.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Lexer/Token.hpp"
#include "Fox/Common/Errors.hpp"

using namespace fox;

static const char* getKindSpelling(TokenKind kind) {
  switch (kind) {
    #define TOKEN(ID) case TokenKind::ID: return #ID;
    #include "Fox/Lexer/TokenKinds.def"
    default: 
      fox_unreachable("unhandled token kind");
  }
}

Token::Token(Kind kind, string_view str, SourceRange range) :
  kind(kind), str(str), range(range) {
  assert(range && "Token constructed with invalid an SourceRange");
}

bool Token::isValid() const {
  return kind != Kind::Invalid;
}

Token::operator bool() const {
  return isValid();
}

bool Token::is(Kind kind) const {
  return (this->kind == kind);
}

void Token::dump(std::ostream& out) const {
  out << str << " [" << getKindSpelling(kind) << "]\n";
}
