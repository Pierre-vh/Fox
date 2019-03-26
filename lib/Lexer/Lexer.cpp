//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : Lexer.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Lexer/Lexer.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"
#include "Fox/Common/SourceManager.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/AST/ASTContext.hpp"
#include "utfcpp/utf8.hpp"
#include <cctype>

using namespace fox;

Lexer::Lexer(ASTContext& astctxt): ctxt(astctxt), 
  diagEngine(ctxt.diagEngine), srcMgr(ctxt.sourceMgr) {}

void Lexer::lexFile(FileID file) {
  assert(file 
    && "invalid FileID!");
  assert((tokens_.size() == 0) 
    && "There are tokens left in the token vector!");
  fileID_ = file;
  string_view content = ctxt.sourceMgr.getFileContent(file);
  // init the iterator/pointers
  fileBeg_ = tokBegPtr_ = curPtr_ = content.begin();
  fileEnd_ = content.end();
  // lex
  lex();
}

TokenVector& Lexer::getTokens() {
  return tokens_; // return empty Token
}

std::size_t Lexer::numTokens() const {
  return tokens_.size();
}

bool Lexer::isEOF() const {
  return (curPtr_ == fileEnd_);
}

void Lexer::resetToken() {
  tokBegPtr_ = curPtr_;
}

void Lexer::lex() {
  FoxChar cur;
  while(!isEOF()) {
    cur = getCurChar();
    switch (cur) {
      // Ignored characters
      case 0:
      case ' ':
      case '\t':
      case '\r':
      case '\n':
      case '\v':
      case '\f':
        // skip them
        advance();
        break;
      // Operators
      case '/':
        // "//" -> Beginning of a line comment
        if(peekNextChar() == '/')
          skipLineComment();
        // "/*" -> Beginning of a block comment
        else if(peekNextChar() == '*')
          skipBlockComment();
        // "/"
        else 
         beginAndPushToken(TokenKind::Slash);
        break;
      case '*':
        resetToken();
        // "**"
        if (peekNextChar() == '*')
          advanceAndPushTok(TokenKind::StarStar);
        // "*"
        else 
          pushTok(TokenKind::Star);
        break;
      case '=': 
        resetToken();
        // "=="
        if(peekNextChar() == '=')
          advanceAndPushTok(TokenKind::EqualEqual);
        // "="
        else 
          pushTok(TokenKind::Equal);
        break;
      case '.':
        beginAndPushToken(TokenKind::Dot);
        break;
      case '+':
        beginAndPushToken(TokenKind::Plus);
        break;
      case '-':
        beginAndPushToken(TokenKind::Minus);
        break;
      case '&':
        resetToken();
        if(peekNextChar() == '&') // "&&" -> Logical And operator
          advanceAndPushTok(TokenKind::AmpAmp);
        else
          pushTok(TokenKind::Invalid);
        break;
      case '|':
        resetToken();
        if(peekNextChar() == '|') // "||" -> Logical Or operator
          advanceAndPushTok(TokenKind::PipePipe);
        else
          pushTok(TokenKind::Invalid);
        break;
      case '%':
        beginAndPushToken(TokenKind::Percent);
        break;
      case '!':
        resetToken();
        if(peekNextChar() == '=') // "!=" -> Inequality operator
          advanceAndPushTok(TokenKind::ExclaimEqual);
        else
          pushTok(TokenKind::Exclaim);
        break;
      case '<':
        resetToken();
        if(peekNextChar() == '=') // "<=" -> Less or Equal
          advanceAndPushTok(TokenKind::LessEqual);
        else
          pushTok(TokenKind::Less);
        break;
      case '>':
        resetToken();
        if(peekNextChar() == '=') // ">=" -> Greater or Equal
          advanceAndPushTok(TokenKind::GreaterEqual);
        else
          pushTok(TokenKind::Greater);
        break;
      // Brackets
      case '(':
        beginAndPushToken(TokenKind::LParen);
        break;
      case ')':
        beginAndPushToken(TokenKind::RParen);
        break;
      case '[':
        beginAndPushToken(TokenKind::LSquare);
        break;
      case ']':
        beginAndPushToken(TokenKind::RSquare);
        break;
      case '{':
        beginAndPushToken(TokenKind::LBrace);
        break;
      case '}':
        beginAndPushToken(TokenKind::RBrace);
        break;
      // Other signs
      case ';':
        beginAndPushToken(TokenKind::Semi);
        break;
      case ':':
        beginAndPushToken(TokenKind::Colon);
        break;
      case ',':
        beginAndPushToken(TokenKind::Comma);
        break;
      // Single/Double quote text
      case '\'':
        lexSingleQuoteText();
        break;
      case '"':
        lexDoubleQuoteText();
        break;
      // Numbers/literals
      case '0': 
      case '1': case '2': case '3':
      case '4': case '5': case '6':
      case '7': case '8': case '9':
        lexIntOrDoubleLiteral();
        break;
      default:
        if(isValidIdentifierHead(cur)) 
          lexIdentifierOrKeyword();
        else
          beginAndPushToken(TokenKind::Invalid);
        break;
    }
  }
}

namespace {
}

void Lexer::lexIdentifierOrKeyword() {
  assert(isValidIdentifierHead(getCurChar()) 
    && "Not a valid identifier head!");
  resetToken();
  while(isValidIdentifierChar(peekNextChar()))
    advance();
  // TODO: Automatically generate this using TokenKinds.def once
  // all token kinds enums are merged.
  string_view str = getCurtokStringView();
  #define KEYWORD(ID, STR) if(str == STR) return pushTok(TokenKind::ID);
  #include "Fox/Lexer/TokenKinds.def"
  pushTok(TokenKind::Identifier);
}

void Lexer::lexIntOrDoubleLiteral() {
  assert(std::isdigit(*curPtr_) && "not a digit");
  // <int_literal> = {(Digit 0 through 9)}
  // <float_literal> = <int_literal> '.' <int_literal>
  resetToken();
  lexIntLiteral();
  // Check if we have a '.' followed by a digit, if so,
  // eat the '.' and call lexIntLiteral()
  if ((*(curPtr_+1) == '.') && std::isdigit(*(curPtr_+2))) {
    curPtr_ += 2;
    lexIntLiteral();
    pushTok(TokenKind::DoubleConstant);
  }
  else 
    pushTok(TokenKind::IntConstant);
}

bool Lexer::lexCharItems(FoxChar delimiter) {
  assert((getCurChar() == delimiter)
    && "current char is not the delimiter!");
  FoxChar cur;
  bool isEscaping = false;
  while (true) {
    // Advance if possible
    if(!advance()) return false;

    // Fetch the current character
    cur = getCurChar();

    // Check for forbidden characters
    if(!canBeCharItem(cur)) 
      return false;

    // If this character is escaped, continue
    if (isEscaping) {
      isEscaping = false;
      continue;
    }

    // Handle the delimiter
    if (cur == delimiter)
      return true;
    // Handle the escape char '\'.
    else if (cur == '\\')
      isEscaping = true;
  }
}

void Lexer::lexSingleQuoteText() {
  assert((getCurChar() == '\'') && "not a quote");
  resetToken();
  // Lex the body of the literal
  bool foundDelimiter = lexCharItems('\'');
  // Check if we were successful.
  if (foundDelimiter) {
    assert((getCurChar() == '\'') 
      && "Found the delimiter but the current char is not the delimiter?");
    // Push the token
    pushTok(TokenKind::SingleQuoteText);
    return;
  }
  diagEngine.report(DiagID::unterminated_char_lit, getCurtokBegLoc());
}

void Lexer::lexDoubleQuoteText() {
  assert((getCurChar() == '"') && "not a double quote");
  resetToken();
  // Lex the body of the literal
  bool foundDelimiter = lexCharItems('"');
  // Check if we were successful.
  if (foundDelimiter) {
    assert((getCurChar() == '"') 
      && "Found the delimiter but the current char is not the delimiter?");
    // Push the token
    pushTok(TokenKind::DoubleQuoteText);
    return;
  }
  diagEngine.report(DiagID::unterminated_str_lit, getCurtokBegLoc());
}

void Lexer::lexIntLiteral() {
  assert(std::isdigit(*curPtr_) && "not a digit");
  // <int_literal> = {(Digit 0 through 9)}
  while (!isEOF()) {
    // Keep incrementing curPtr until the next char
    // isn't a digit.
    if(std::isdigit(*(curPtr_+1)))
      ++curPtr_;
    // if the next char isn't a digit, stop.
    else break;
  }
}

void Lexer::skipLineComment() {
  // <line_comment> = '/' '/' (any character except '\n')
  assert((getCurChar() == '/') && "not a comment");
  while (char cur = *(curPtr_++)) {
    if(curPtr_ == fileEnd_) return;
    if (cur == '\n') return;
  }
}

void Lexer::skipBlockComment() {
  // <block_comment> = '/' '*' (any character except ('*' + '/'))
  assert(((*curPtr_) == '/') 
    && "not a comment");
  const char* beg = curPtr_;
  while (char cur = *(curPtr_++)) {
    if (curPtr_ == fileEnd_) {
      diagEngine.report(DiagID::unterminated_block_comment, getLocOfPtr(beg));
      return;
    }
    if ((cur == '*') && ((*curPtr_) == '/')) {
      ++curPtr_;
      return;
    }
  }
}

bool Lexer::canBeCharItem(FoxChar c) const {
  switch (c) {
    // Newlines are forbidden
    case '\n': case '\r':
      return false;
    default:
      return true;
  }
}

bool Lexer::isValidIdentifierHead(FoxChar ch) const {
  // <identifier_head> = '_' | (Upper/Lowercase letter A through Z)
  if(ch == '_') return true;
  if((ch >= 'a') && (ch <= 'z')) return true;
  if((ch >= 'A') && (ch <= 'Z')) return true;
  return false;
}

bool Lexer::isValidIdentifierChar(FoxChar ch) const {
  if(isValidIdentifierHead(ch)) return true;
  if((ch >= '0') && (ch <= '9')) return true;
  return false;
}

FoxChar Lexer::getCurChar() const {
  return utf8::peek_next(curPtr_, fileEnd_);
}

FoxChar Lexer::peekNextChar() const {
  auto it = curPtr_;
  utf8::advance(it, 1, fileEnd_);
  if(it != fileEnd_)
    return utf8::peek_next(it, fileEnd_);
  return 0;
}

bool Lexer::advance() {
  utf8::next(curPtr_, fileEnd_);
  return !isEOF();
}

SourceLoc Lexer::getLocOfPtr(const char* ptr) const {
  return SourceLoc(fileID_, std::distance(fileBeg_, ptr));
}

SourceLoc Lexer::getCurPtrLoc() const {
  return getLocOfPtr(curPtr_);
}

SourceLoc Lexer::getCurtokBegLoc() const {
  return getLocOfPtr(tokBegPtr_);
}

SourceRange Lexer::getCurtokRange() const {
  SourceRange range;
  if(curPtr_ == tokBegPtr_)
    range = SourceRange(getCurPtrLoc());
  else 
    range = SourceRange(getCurtokBegLoc(), getCurPtrLoc());
  assert(range && "invalid location information");
  return range;
}

string_view Lexer::getCurtokStringView() const {
  if(curPtr_ == fileEnd_) return string_view(curPtr_, 0);
  const char* it = curPtr_;
  utf8::advance(it, 1, fileEnd_);
  return string_view(tokBegPtr_, std::distance(tokBegPtr_, it));
}
