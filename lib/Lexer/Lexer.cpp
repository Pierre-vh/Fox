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
using Tok = Token::Kind;

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
        // "/" -> A slash
        else 
         beginAndPushToken(SignType::S_SLASH);
        break;
      case '*':
        resetToken();
        // "**" -> Exponent operator
        if (peekNextChar() == '*')
          advanceAndPushTok(SignType::S_OP_EXP);
        // "*" -> Asterisk
        else 
          pushTok(SignType::S_ASTERISK);
        break;
      case '=': 
        resetToken();
        // "==" -> Equality operator
        if(peekNextChar() == '=')
          advanceAndPushTok(SignType::S_OP_EQ);
        // "=" -> Equal
        else 
          pushTok(SignType::S_EQUAL);
        break;
      case '.':
        beginAndPushToken(SignType::S_DOT);
        break;
      case '+':
        beginAndPushToken(SignType::S_PLUS);
        break;
      case '-':
        beginAndPushToken(SignType::S_MINUS);
        break;
      case '&':
        resetToken();
        if(peekNextChar() == '&') // "&&" -> Logical And operator
          advanceAndPushTok(SignType::S_OP_LAND);
        else
          pushTok(Tok::Invalid);
        break;
      case '|':
        resetToken();
        if(peekNextChar() == '|') // "||" -> Logical Or operator
          advanceAndPushTok(SignType::S_OP_LOR);
        else
          pushTok(Tok::Invalid);
        break;
      case '%':
        beginAndPushToken(SignType::S_PERCENT);
        break;
      case '!':
        resetToken();
        if(peekNextChar() == '=') // "!=" -> Inequality operator
          advanceAndPushTok(SignType::S_OP_INEQ);
        else
          pushTok(SignType::S_EXCL_MARK);
        break;
      case '<':
        resetToken();
        if(peekNextChar() == '=') // "<=" -> Less or Equal
          advanceAndPushTok(SignType::S_OP_LTEQ);
        else
          pushTok(SignType::S_LESS_THAN);
        break;
      case '>':
        resetToken();
        if(peekNextChar() == '=') // ">=" -> Greater or Equal
          advanceAndPushTok(SignType::S_OP_GTEQ);
        else
          pushTok(SignType::S_GREATER_THAN);
        break;
      // Brackets
      case '(':
        beginAndPushToken(SignType::S_ROUND_OPEN);
        break;
      case ')':
        beginAndPushToken(SignType::S_ROUND_CLOSE);
        break;
      case '[':
        beginAndPushToken(SignType::S_SQ_OPEN);
        break;
      case ']':
        beginAndPushToken(SignType::S_SQ_CLOSE);
        break;
      case '{':
        beginAndPushToken(SignType::S_CURLY_OPEN);
        break;
      case '}':
        beginAndPushToken(SignType::S_CURLY_CLOSE);
        break;
      // Other signs
      case ';':
        beginAndPushToken(SignType::S_SEMICOLON);
        break;
      case ':':
        beginAndPushToken(SignType::S_COLON);
        break;
      case ',':
        beginAndPushToken(SignType::S_COMMA);
        break;
      // char/string literals
      case '\'':
        lexCharLiteral();
        break;
      case '"':
        lexStringLiteral();
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
          lexMaybeReservedIdentifier();
        else
          beginAndPushToken(Tok::Invalid);
        break;
    }
  }
}

void Lexer::lexMaybeReservedIdentifier() {
  assert(isValidIdentifierHead(getCurChar()) 
    && "Not a valid identifier head!");
  resetToken();
  while(isValidIdentifierChar(peekNextChar()))
    advance();
  // TODO: ID reserved identifiers/keyword.
  pushTok(Tok::Identifier);
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
    pushTok(Tok::DoubleLiteral);
  }
  else 
    pushTok(Tok::IntLiteral);
}

bool Lexer::lexCharItem() {
  // <char_item> = Any non-space unicode character except '
  //             | "\n" | "\r" | "\t" | "\\"
  // If the current character is a quote, the literal is empty.
  // Don't eat the quote (to not trigger a missing quote error)
  // and diagnose it.
  FoxChar cur = getCurChar();
  if (cur == '\'') {
    diagEngine.report(DiagID::empty_char_lit, getCurtokRange());
    return false;
  }
  // Check for escape sequences
  if (cur == '\\') {
    // Save the pointer to the backlash.
    const char* backslashPtr = curPtr_;
    // Skip it
    advance();
    // Check for valid escape sequences.
    switch (getCurChar()) {
      case '\\':
      case 'n':
      case 'r':
      case 't':
        advance(); // skip them and we're done.
        return true;
      default:
        // eat all subsequent alphanumeric characters so we can recover.
        while(!isEOF() && std::isalnum(*(++curPtr_)));
        // prepare a diagnostic at the backlash's loc, with the
        // extra range highlighting the rest of the escape sequence.
        const char* escapeSeqEndPtr = (curPtr_-1);
        SourceRange extraRange(getLocOfPtr(backslashPtr+1),
                               getLocOfPtr(escapeSeqEndPtr));
        diagEngine
          .report(DiagID::unknown_escape_seq, getLocOfPtr(backslashPtr))
          .setExtraRange(extraRange);
        // As the escape sequence is invalid, return false.
        return false;
    }
  }
  // Check for other forbidden characters
  if (isBannedTextItem(cur)) return false;
  if (cur == '\t') return false;
  // Else we should be good.
  advance();
  return true;
}

void Lexer::lexCharLiteral() {
  assert(((*curPtr_) == '\'') && "not a quote");
  // <char_literal> = ''' <char_item> '''
  resetToken();
  // Skip the '
  ++curPtr_; 
  // Lex the body of the literal
  bool succ = lexCharItem();
  // Find the closing quote. If we have it, push the token. if we don't,
  // diagnose it.
  if ((*curPtr_) == '\'') {
    // Only push successful tokens.
    if(succ)
      pushTok(Tok::CharLiteral);
    // If the literal isn't valid, eat the quote and reset the token.
    else {
      ++curPtr_;
      resetToken();
    }
    return;
  }
  diagEngine.report(DiagID::unterminated_char_lit, getCurtokBegLoc());
}

// TODO - UNFINISHED
bool Lexer::lexStringItem() {
  // <string_item> = '\"' | Any unicode character except '\r', '\n' and "
  bool escaped = false;
  while (!isEOF()) {
    // Check for forbidden characters
    // Otherwise, ignore escaped characters
    if(escaped) escaped = false;
    else {

    }
  }
  // Return true except if we reached EOF.
  return !isEOF();
}

void Lexer::lexStringLiteral() {
  assert(((*curPtr_) == '"') && "not a string");
  // <string_literal> = '"' {<string_item>} '"'
  resetToken();
  ++curPtr_; // skip the '"'
  // Lex the body of the litera
  bool succ = lexStringItem();
  if ((*curPtr_) == '"') {
    if(succ)
      pushTok(Tok::StringLiteral);
    // If the literal isn't valid, eat the quote and reset the token.
    else {
      ++curPtr_;
      resetToken();
    }
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
    else 
      break;
  }
}

void Lexer::skipLineComment() {
  // <line_comment> = '/' '/' (any character except '\n')
  assert(((*curPtr_) == '/') 
    && "not a comment");
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

bool Lexer::isBannedTextItem(char c) const {
  // <banned_text_item>  = 0x0a (LF) | 0x0b (VT) | 0x0c (FF) | 0x0d (CR)
  switch (c) {
    case '\n': case '\r': case '\v': case '\f':
      return true;
    default:
      return false;
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
