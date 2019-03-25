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

using namespace fox;
using TKind = Token::Kind;

Lexer::Lexer(ASTContext& astctxt): ctxt(astctxt), 
  diagEngine(ctxt.diagEngine), srcMgr(ctxt.sourceMgr) {}

void Lexer::lexFile(FileID file) {
  assert(file 
    && "INVALID FileID!");
  assert((tokens_.size() == 0) 
    && "There are tokens left in the token vector!");
  fileID_ = file;
  string_view content = ctxt.sourceMgr.getFileContent(file);
  // init the iterator/pointers
  fileBeg_ = tokBegPtr_ = curPtr_ = content.begin();
  fileEnd_ = content.end();
}

TokenVector& Lexer::getTokenVector() {
  return tokens_; // return empty Token
}

std::size_t Lexer::resultSize() const {
  return tokens_.size();
}

FileID Lexer::getCurrentFile() const {
  return fileID_;
}

void Lexer::beginToken() {
  tokBegPtr_ = curPtr_;
}

void Lexer::lex() {
  FoxChar cur;
  do {
    cur = getCurChar();
    switch (cur) {
      // Spaces/ignored characters
      case ' ':
      case '\t':
      case '\r':
      case '\n':
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
        beginToken();
        // "**" -> Exponent operator
        if (peekNextChar() == '*')
          advanceAndPushTok(SignType::S_OP_EXP);
        // "*" -> Asterisk
        else 
          pushTok(SignType::S_ASTERISK);
        break;
      case '=': 
        beginToken();
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
        beginToken();
        if(peekNextChar() == '&') // "&&" -> Logical And operator
          advanceAndPushTok(SignType::S_OP_LAND);
        else
          pushTok(TKind::Invalid);
        break;
      case '|':
        beginToken();
        if(peekNextChar() == '|') // "||" -> Logical Or operator
          advanceAndPushTok(SignType::S_OP_LOR);
        else
          pushTok(TKind::Invalid);
        break;
      case '%':
        beginAndPushToken(SignType::S_PERCENT);
        break;
      case '!':
        beginToken();
        if(peekNextChar() == '=') // "!=" -> Inequality operator
          advanceAndPushTok(SignType::S_OP_INEQ);
        else
          pushTok(SignType::S_EXCL_MARK);
        break;
      case '<':
        beginToken();
        if(peekNextChar() == '=') // "<=" -> Less or Equal
          advanceAndPushTok(SignType::S_OP_LTEQ);
        else
          pushTok(SignType::S_LESS_THAN);
        break;
      case '>':
        beginToken();
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
          beginAndPushToken(TKind::Invalid);
        break;
    }
  // Keep going until we run out of codepoints to evaluate
  } while(advance());
}

void Lexer::lexIdentifierOrKeyword() {
}

void Lexer::lexIntOrDoubleLiteral() {
}

void Lexer::lexIntLiteral() {
}

void Lexer::skipLineComment() {

}

void Lexer::skipBlockComment() {

}

bool Lexer::isValidIdentifierHead(FoxChar ch) const {
  // <identifier_head> = '_' | (Upper/Lowercase letter A through Z)
  if(ch == '_') return true;
  if((ch >= 'a') && (ch <= 'z')) return true;
  if((ch >= 'A') && (ch <= 'Z')) return true;
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
  return (curPtr_ != fileEnd_);
}

SourceLoc Lexer::getCurPtrLoc() const {
  return SourceLoc(fileID_, std::distance(fileBeg_, curPtr_));
}

SourceLoc Lexer::getCurtokBegLoc() const {
  return SourceLoc(fileID_, std::distance(fileBeg_, tokBegPtr_));
}

SourceRange Lexer::getCurtokRange() const {
  SourceRange range(getCurtokBegLoc(), getCurPtrLoc());
  assert(range && "invalid location information");
  return range;
}

string_view Lexer::getCurtokStringView() const {
  if(curPtr_ == fileEnd_) return string_view(curPtr_, 0);
  const char* it = curPtr_;
  utf8::advance(it, 1, fileEnd_);
  return string_view(tokBegPtr_, std::distance(tokBegPtr_, it));
}
