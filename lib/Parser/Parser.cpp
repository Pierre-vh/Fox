//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : Parser.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Parser/Parser.hpp"
#include "Fox/AST/ASTContext.hpp"
#include "Fox/AST/Types.hpp"
#include "Fox/Common/SourceManager.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"
#include "Fox/Lexer/Lexer.hpp"

using namespace fox;

//----------------------------------------------------------------------------//
// Parser
//----------------------------------------------------------------------------//

Parser::Parser(ASTContext& ctxt, Lexer& lexer, UnitDecl *unit):
  ctxt(ctxt), lexer(lexer), srcMgr(ctxt.sourceMgr), diagEngine(ctxt.diagEngine),
  curDeclCtxt_(unit) {
  tokenIterator_ = getTokens().begin();
  isAlive_ = true;
}

std::pair<Identifier, SourceRange> Parser::consumeIdentifier() {
  Token tok = getCurtok();
  assert(isCurTokAnIdentifier() && "not an identifier");
  Identifier id = ctxt.getIdentifier(tok.str);
  consume();
  return std::make_pair(id, tok.range);
}

SourceRange Parser::consume() {
  auto tok = getCurtok();
  assert(tok && "Consuming EOF token");
  if (tokenIterator_ != getTokens().end()) tokenIterator_++;
  return tok.range;
}

SourceRange Parser::tryConsume(TokenKind kind) {
  if(getCurtok().is(kind)) 
    return consume();
  return SourceRange();
}

Parser::Result<TypeLoc> Parser::parseBuiltinTypename() {
  // <builtin_type_name>   = "int" | "float" | "bool" | "string" | "char"
  using RtrTy = Result<TypeLoc>;

  // "int"
  if (auto range = tryConsume(TokenKind::IntKw))
    return RtrTy(TypeLoc(PrimitiveType::getInt(ctxt), range));
  
  // "float"
  if (auto range = tryConsume(TokenKind::DoubleKw))
    return RtrTy(TypeLoc(PrimitiveType::getDouble(ctxt), range));

  // "bool"
  if (auto range = tryConsume(TokenKind::BoolKw))
    return RtrTy(TypeLoc(PrimitiveType::getBool(ctxt), range));

  // "string"
  if (auto range = tryConsume(TokenKind::StringKw))
    return RtrTy(TypeLoc(PrimitiveType::getString(ctxt), range));

  // "char"
  if (auto range = tryConsume(TokenKind::CharKw))
    return RtrTy(TypeLoc(PrimitiveType::getChar(ctxt), range));

  return RtrTy::NotFound();
}

Parser::Result<TypeLoc> Parser::parseType() {
  // <type> =  ('[' <type> ']') | <builtin_type_name>
  // ('[' <type> ']')
  if(auto lsquare = tryConsume(TokenKind::LSquare).getBeginLoc()) {
    auto ty_res = parseType();
    bool error = !ty_res;
    if(error && ty_res.isNotFound())
      reportErrorExpected(DiagID::expected_type);
    auto rsquare = tryConsume(TokenKind::RSquare).getBeginLoc();
    if(!rsquare) {
      error = true;
      reportErrorExpected(DiagID::expected_rbracket);
      bool resync = skipUntil(TokenKind::RSquare, 
                             /*stop@semi*/ true, /*consume*/ false);
      if(resync)
        rsquare = tryConsume(TokenKind::RSquare).getBeginLoc();
    }
    if(error)
      return Result<TypeLoc>::Error();
    
    SourceRange range(lsquare, rsquare);
    assert(range && "range should be valid");
    Type ty = ty_res.get().getType();
    ty = ArrayType::get(ctxt, ty);
    return Result<TypeLoc>(TypeLoc(ty, range));
  }
  // <builtin_type_name> 
  if (auto ty_res = parseBuiltinTypename()) 
    return ty_res;
  return Result<TypeLoc>::NotFound();
}

Token Parser::getCurtok() const {
  if (!isDone())
    return *tokenIterator_;
  return Token();
}

Token Parser::getPreviousToken() const {
  auto it = tokenIterator_;
  if (it != getTokens().begin())
    return *(--it);
  return Token();
}

TokenVector& Parser::getTokens() {
  return lexer.getTokens();
}

const TokenVector& Parser::getTokens() const {
  return lexer.getTokens();
}

void Parser::skip() {
  Token tok = getCurtok();
  assert(tok && "Skipping EOF Token");
  consume();
  switch (tok.kind) {
    case TokenKind::LBrace:
      skipUntil(TokenKind::RBrace, /*stopAtSemi*/ false, /*consume*/ true);
      break;
    case TokenKind::LParen:
      skipUntil(TokenKind::RParen, /*stopAtSemi*/ false, /*consume*/ true);
      break;
    case TokenKind::LSquare:
      skipUntil(TokenKind::RSquare, /*stopAtSemi*/ false, /*consume*/ true);
      break;
  }
}

bool
Parser::skipUntil(TokenKind kind, bool stopAtSemi, bool shouldConsumeToken) {
  while (!isDone()) {
    Token curtok = getCurtok();
    assert(curtok && "curtok is invalid");
    // Match the desired token
    if (curtok.is(kind)) {
      if(shouldConsumeToken)
        consume();
      return true;
    }
    // Stop at semi if required
    if(stopAtSemi && curtok.is(TokenKind::Semi))
      return false;
    // Else, skip
    skip();
  }
  die();
  return false;
}

bool Parser::skipToNextDecl() {
  while(!isDone()) {
    auto tok = getCurtok();
    // if it's let/var/func, return.
    if (tok.is(TokenKind::FuncKw) 
     || tok.is(TokenKind::LetKw) 
     || tok.is(TokenKind::VarKw))
      return true;
    // else, keep skipping.
    skip();
  }
  // If we get here, we reached eof.
  die();
  return false;
}

void Parser::die() {
  tokenIterator_ = getTokens().end();
  isAlive_ = false;
}

Diagnostic Parser::reportErrorExpected(DiagID diag) {
  SourceRange errorRange;
  if (Token prevTok = getPreviousToken()) {
    SourceLoc loc = prevTok.range.getEndLoc();
    // Get the next character in the file. This will be our 
    // error's location.
    loc = srcMgr.advance(loc);
    errorRange = SourceRange(loc);
  }
  else {
    // No valid undo token, use the current token's range as the 
    // error location. (This case should be fairly rare, 
		// or never happen at all. tests needed)
    Token curTok = getCurtok();
    assert(curTok && "No valid previous token and no valid current token?");
    errorRange = curTok.range;
  }
  return diagEngine.report(diag, errorRange);
}

bool Parser::isDone() const {
  return (tokenIterator_ == getTokens().end()) || (!isAlive());
}

bool Parser::isAlive() const {
  return isAlive_;
}

DeclContext* Parser::getCurrentDeclCtxt() const {
  return curDeclCtxt_;
}

bool Parser::isCurTokAnIdentifier() const {
  return getCurtok().is(TokenKind::Identifier);
}

//----------------------------------------------------------------------------//
// Parser::RAIIDeclCtxt
//----------------------------------------------------------------------------//

Parser::RAIIDeclCtxt::RAIIDeclCtxt(Parser *p, DeclContext* dc):
  parser_(p) {
  assert(p && "Parser instance can't be nullptr");
  lastDC_ = p->curDeclCtxt_;
  p->curDeclCtxt_ = dc;
}

void Parser::RAIIDeclCtxt::restore() {
  assert(parser_ && "Parser instance can't be nullptr");
  parser_->curDeclCtxt_ = lastDC_;
  parser_ = nullptr;
}

Parser::RAIIDeclCtxt::~RAIIDeclCtxt() {
  if(parser_) // parser_ will be nullptr if we restored early
    restore();
}

//----------------------------------------------------------------------------//
// Parser::DelayedDeclRegistration
//----------------------------------------------------------------------------//

Parser::DelayedDeclRegistration::DelayedDeclRegistration(Parser* p)
  : parser_(p) {
  assert(parser_ && "parser is nullptr!");
  prevCurDDR_ = parser_->curDDR_;
  parser_->curDDR_ = this;
}

Parser::DelayedDeclRegistration::~DelayedDeclRegistration() {
  abandon();
}

void Parser::DelayedDeclRegistration::addDecl(Decl* decl) {
  decls_.push_back(decl);
}

void Parser::DelayedDeclRegistration::abandon() {
  if (parser_) {
    // Restore the previous DDR
    parser_->curDDR_ = prevCurDDR_;
    // Set the parser instance to nullptr, so
    // we don't abandon/complete twice.
    parser_ = nullptr;
    // Clear the decls vector.
    decls_.clear(); 
  }
}

void Parser::DelayedDeclRegistration::complete(ScopeInfo scope) {
  assert(parser_ && "transaction has already been completed/abandoned!");
  assert(scope && "Cannot complete a transaction with a null ScopeInfo!");
  for(auto decl : decls_)
    parser_->registerDecl(decl, scope);
  abandon();
}