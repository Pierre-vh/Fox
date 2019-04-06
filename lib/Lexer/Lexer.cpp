//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : Lexer.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// Implements the Token and Lexer classes
//----------------------------------------------------------------------------//

#include "Fox/Lexer/Lexer.hpp"
#include "Fox/Lexer/Token.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"
#include "Fox/Common/SourceManager.hpp"
#include "Fox/Common/Errors.hpp"
#include "utfcpp/utf8.hpp"
#include <cctype>

using namespace fox;

//----------------------------------------------------------------------------//
// Token
//----------------------------------------------------------------------------//

static const char* getKindSpelling(TokenKind kind) {
  switch (kind) {
    #define TOKEN(ID) case TokenKind::ID: return #ID;
    #include "Fox/Lexer/TokenKinds.def"
    default: 
      fox_unreachable("unhandled token kind");
  }
}

Token::Token(Kind kind, string_view str, SourceRange range) :
  kind(kind), str(str), range(range) {}

bool Token::isValid() const {
  return kind != Kind::Invalid;
}

bool Token::isEOF() const {
  return kind == Kind::EndOfFile;
}

Token::operator bool() const {
  return isValid();
}

bool Token::is(Kind kind) const {
  return (this->kind == kind);
}

void Token::dump(std::ostream& out) const {
  if(str.size())
    out << '"' << str << "\", ";
  out << getKindSpelling(kind) << "\n";
}

void Token::dump(std::ostream& out, SourceManager& srcMgr, 
                 bool printFileName) const {
  if(str.size())
    out << '"' << str << "\", ";
  out << getKindSpelling(kind);
  if(range)
    out << ", " << srcMgr.getCompleteRange(range).toString(printFileName);
  out << '\n';
}

//----------------------------------------------------------------------------//
// Lexer
//----------------------------------------------------------------------------//

Lexer::Lexer(SourceManager& srcMgr, DiagnosticEngine& diags, FileID file) : 
  theFile(file), diagEngine(diags), sourceMgr(srcMgr) {
  assert(theFile && "File isn't valid!");
}

void Lexer::lex() {
  assert((tokens_.size() == 0) 
    && "There are tokens in the token vector!");
  string_view content = sourceMgr.getFileContent(theFile);
  // init the iterator/pointers
  fileBeg_ = tokBegPtr_ = curPtr_ = content.begin();
  fileEnd_ = content.end();
  assert(fileBeg_ && tokBegPtr_ && curPtr_ && fileEnd_ 
    && "Iterators are null");
  // Do the actual lexing
  lexImpl();
  assert(isEOF() && "Char lefts in the input");
  // Append the EOF token
  tokens_.push_back(Token(TokenKind::EndOfFile, string_view(), SourceRange()));
}

TokenVector& Lexer::getTokens() {
  assert(tokens_.size() && "lex() has not been called!");
  assert(tokens_.back().isEOF() && "last token is not EOF");
  return tokens_; // return empty Token
}

SourceLoc Lexer::getLocFromPtr(const char* ptr) const {
  assert(ptr 
    && "pointer is null!");
  assert((ptr >= fileBeg_) && (ptr <= fileEnd_)
    && "Pointer not contained in the current file's buffer");
  return SourceLoc(theFile, std::distance(fileBeg_, ptr));
}

SourceRange Lexer::getRangeFromPtrs(const char* a, const char* b) const {
  return SourceRange(getLocFromPtr(a), getLocFromPtr(b));
}

std::size_t Lexer::numTokens() const {
  return tokens_.size();
}

void Lexer::pushTok(TokenKind kind) {
  tokens_.push_back(Token(kind, getCurtokStringView(), getCurtokRange()));
  advance();
  resetToken();
}

void Lexer::beginAndPushToken(TokenKind kind) {
  resetToken();
  pushTok(kind);
}

void Lexer::advanceAndPushTok(TokenKind kind) {
  advance();
  pushTok(kind);
}

bool Lexer::isEOF() const {
  return (curPtr_ == fileEnd_);
}

void Lexer::resetToken() {
  tokBegPtr_ = curPtr_;
}

void Lexer::lexImpl() {
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
        lexIntOrDoubleConstant();
        break;
      default:
        if(isValidIdentifierHead(cur)) 
          lexIdentifierOrKeyword();
        else {
          // Unknown/Invalid character, diagnose it.
          diagEngine.report(DiagID::invalid_char_in_sf, getCurPtrLoc());
          advance();
        }
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
  // Lex every character
  while(isValidIdentifierChar(peekNextChar()))
    advance();
  // Identify keywords
  string_view str = getCurtokStringView();
  #define KEYWORD(ID, STR) if(str == STR) return pushTok(TokenKind::ID);
  #include "Fox/Lexer/TokenKinds.def"
  // Else it's just an identifier.
  pushTok(TokenKind::Identifier);
}

void Lexer::lexIntOrDoubleConstant() {
  assert(std::isdigit(*curPtr_) && "not a digit");
  // <int_literal> = {(Digit 0 through 9)}
  // <float_literal> = <int_literal> '.' <int_literal>
  resetToken();
  lexIntConstant();
  // Check if we have a '.' followed by a digit, if so,
  // eat the '.' and call lexIntConstant()
  if ((*(curPtr_+1) == '.') && std::isdigit(*(curPtr_+2))) {
    curPtr_ += 2;
    lexIntConstant();
    pushTok(TokenKind::DoubleConstant);
  }
  else 
    pushTok(TokenKind::IntConstant);
}

// <string_item>    = Any character except the double quote ", 
//                    backslash or newline character | <escape_seq> 
// <string_literal> = '"' {<string_item>} '"'
// <char_item>      = Any character except the single quote ',
//                    backslash or newline character | <escape_seq> 
// <char_literal>   = ''' {<char_item>} '''
//
// The only difference between <char_item> and <string_item> is 
// the forbidden delimiter, so we can use the same lexing
// method. We just pass the delimiter as parameter to know
// when to stop.
bool Lexer::lexTextItems(FoxChar delimiter) {
  assert(((delimiter == '\'') || (delimiter == '"'))
    && "unknown/unsupported delimiter");
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
    if(cur == '\n') 
      return false;
    if((cur == '\r') && (peekNextChar() == '\n'))
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

void Lexer::lexCharLiteral() {
  assert((getCurChar() == '\'') && "not a quote");
  // <char_literal> = ''' {<char_item>} '''
  resetToken();
  // Lex the body of the literal
  bool foundDelimiter = lexTextItems('\'');
  // Check if we were successful.
  if (foundDelimiter) {
    assert((getCurChar() == '\'') 
      && "Found the delimiter but the current char is not the delimiter?");
    // Push the token
    pushTok(TokenKind::CharLiteral);
    return;
  }
  diagEngine.report(DiagID::unterminated_char_lit, getCurtokBegLoc());
}

void Lexer::lexStringLiteral() {
  assert((getCurChar() == '"') && "not a double quote");
  resetToken();
  // Lex the body of the literal
  bool foundDelimiter = lexTextItems('"');
  // Check if we were successful.
  if (foundDelimiter) {
    assert((getCurChar() == '"') 
      && "Found the delimiter but the current char is not the delimiter?");
    // Push the token
    pushTok(TokenKind::StringLiteral);
    return;
  }
  diagEngine.report(DiagID::unterminated_str_lit, getCurtokBegLoc());
}

void Lexer::lexIntConstant() {
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
      SourceLoc loc = getLocFromPtr(beg);
      diagEngine.report(DiagID::unterminated_block_comment, loc);
      return;
    }
    if ((cur == '*') && ((*curPtr_) == '/')) {
      ++curPtr_;
      return;
    }
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
  // If the current character isn't an utf8 character,
  // just return *curPtr_.
  if(!((*curPtr_) & 0x80)) return (*curPtr_);
  return utf8::peek_next(curPtr_, fileEnd_);
}

FoxChar Lexer::peekNextChar() const {
  // TODO: Cache this somewhere.
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

SourceLoc Lexer::getCurPtrLoc() const {
  return getLocFromPtr(curPtr_);
}

SourceLoc Lexer::getCurtokBegLoc() const {
  return getLocFromPtr(tokBegPtr_);
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
