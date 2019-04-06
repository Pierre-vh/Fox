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
}

FileID Parser::getFileID() const {
  return lexer.theFile;
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
  assert(!isDone() && "Consuming EOF token");
  tokenIterator_++;
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
    // <type>
    auto typeResult = parseType();
    if(typeResult.isNotFound())
      reportErrorExpected(DiagID::expected_type);
    TypeLoc typeloc = typeResult.get();

    // ']'
    auto rsquare = tryConsume(TokenKind::RSquare).getBeginLoc();
    if(!rsquare) {
      reportErrorExpected(DiagID::expected_rbracket);
      diagEngine.report(DiagID::to_match_this_bracket, lsquare);
      return Result<TypeLoc>::Error();
    }

    // Can't build the ArrayType if we don't have a type.
    if(!typeloc.isTypeValid())
      return Result<TypeLoc>::Error();

    SourceRange range(lsquare, rsquare);
    assert(range && "range should be valid");
    Type type = ArrayType::get(ctxt, typeloc.getType());
    return Result<TypeLoc>(TypeLoc(type, range));
  }
  // <builtin_type_name> 
  if (auto typeResult = parseBuiltinTypename()) 
    return typeResult;
  return Result<TypeLoc>::NotFound();
}

Token Parser::getCurtok() const {
  return *tokenIterator_;
}

Token Parser::getPreviousToken() const {
  auto it = tokenIterator_;
  if (it != getTokens().begin())
    return *(--it);
  return Token();
}

bool Parser::isStartOfDecl(const Token& tok) {
  switch (tok.kind) {
    case TokenKind::FuncKw:
    case TokenKind::VarKw:
    case TokenKind::LetKw:
      return true;
    default:
      return false;
  }
}

bool Parser::isStartOfStmt(const Token & tok) {
  switch (tok.kind) {
    case TokenKind::VarKw:
    case TokenKind::LetKw:
    case TokenKind::IfKw:
    case TokenKind::WhileKw:
    case TokenKind::ReturnKw:
      return true;
    default:
      return false;
  }
}

TokenVector& Parser::getTokens() {
  return lexer.getTokens();
}

const TokenVector& Parser::getTokens() const {
  return lexer.getTokens();
}

void Parser::skip() {
  Token tok = getCurtok();
  assert(!tok.isEOF() && "Skipping EOF Token");
  consume();
  switch (tok.kind) {
    case TokenKind::LBrace:
      skipUntil(TokenKind::RBrace);
      consume();
      break;
    case TokenKind::LParen:
      skipUntil(TokenKind::RParen);
      consume();
      break;
    case TokenKind::LSquare:
      skipUntil(TokenKind::RSquare);
      consume();
      break;
  }
}

bool
Parser::skipUntil(TokenKind kind) {
  while (!isDone()) {
    Token curtok = getCurtok();
    assert(curtok && "curtok is invalid");
    // Stop at the desired token
    if (curtok.is(kind))
      return true;
    // Else skip the next stmt/block.
    skip();
  }
  return false;
}

bool Parser::skipUntilStmt() {
  while(!isDone()) {
    Token tok = getCurtok();
    if (isStartOfStmt(tok) || tok.is(TokenKind::RBrace))
      return true;
    // Stop when we find a semicolon and consume it.
    // Consider this a success if the parser is not done because
    // the next token is always the beginning of a statement.
    if (tok.is(TokenKind::Semi)) {
      consume();
      return !isDone();
    }
    skip();
  }
  return false;
}

bool Parser::skipUntilDeclStmtOr(TokenKind kind) {
  while (!isDone()) {
    Token tok = getCurtok();
    assert(tok && "curtok is invalid");
    // Match the desired token
    if (tok.is(kind))
      return true;
    // Stop when we find a semicolon and consume it (so the current token
    // is "guaranteed" to be EOF or to begin a statement)
    if (tok.is(TokenKind::Semi)) {
      consume();
      return false;
    }
    // Stop at the start of statements, or if the token is a RBrace '}'
    if(isStartOfStmt(tok) || tok.is(TokenKind::RBrace)) 
      return false;
    // Also, stop at the start of decls.
    if(isStartOfDecl(tok))
      return false;
    // else, skip the token/block.
    skip();
  }
  return false;
}

bool Parser::skipUntilDecl() {
  while(!isDone()) {
    if (isStartOfDecl(getCurtok()))
      return true;
    skip();
  }
  return false;
}

Diagnostic Parser::reportErrorExpected(DiagID diag) {
  SourceRange errorRange;
  if (Token prevTok = getPreviousToken()) {
    SourceLoc loc = prevTok.range.getEndLoc();
    // The first character after our Token will be the error's location.
    loc = srcMgr.advance(loc);
    errorRange = SourceRange(loc);
  }
  else {
    // No valid previous token
    // FIXME: Is this case even possible?
    Token curTok = getCurtok();
    assert(curTok && "No valid previous token and no valid current token?");
    errorRange = SourceRange(curTok.range.getBeginLoc());
  }
  return diagEngine.report(diag, errorRange);
}

bool Parser::isDone() const {
  return getCurtok().isEOF();
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