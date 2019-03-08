//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : Parser.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Parser/Parser.hpp"
#include "Fox/AST/ASTContext.hpp"
#include "Fox/Common/SourceManager.hpp"
#include "Fox/AST/Types.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"

using namespace fox;

//----------------------------------------------------------------------------//
// Parser
//----------------------------------------------------------------------------//

Parser::Parser(ASTContext& ctxt, TokenVector& l, UnitDecl *unit):
  ctxt(ctxt), tokens(l), srcMgr(ctxt.sourceMgr), diagEngine(ctxt.diagEngine),
  curDeclCtxt_(unit) {
  tokenIterator_ = tokens.begin();
  isAlive_ = true;
}

Optional<std::pair<Identifier, SourceRange>>
Parser::consumeIdentifier() {
  Token tok = getCurtok();
  if (tok.isIdentifier()) {
    Identifier id = tok.getIdentifier();
    assert(id && "Token's an identifier but contains a null Identifier?");
    next();
    return std::make_pair(id, tok.getSourceRange());
  }
  return None;
}

SourceLoc Parser::consumeSign(SignType s) {
  assert(!isBracket(s) 
		&& "This method shouldn't be used to match brackets ! "
			  "Use consumeBracket instead!");
  auto tok = getCurtok();
  if (tok.is(s)) {
    next();
    return tok.getSourceRange().getBeginLoc();
  }
  return {};
}

SourceLoc Parser::consumeBracket(SignType s) {
  assert(isBracket(s) 
	&& "This method should only be used on brackets! "
	  	"Use consumeSign to match instead!");
  auto tok = getCurtok();
  // Lambda to diagnose an overflow and kill the parser.
  auto diagnoseOverflow = [&](DiagID id) {
    diagEngine.report(id, tok.getSourceRange());
    die();
  };
  if (tok.is(s)) {
    switch (s) {
      case SignType::S_CURLY_OPEN:
        if (curlyBracketsCount_ < maxBraceDepth_)
          curlyBracketsCount_++;
        else
          diagnoseOverflow(DiagID::curly_bracket_overflow);
        break;
      case SignType::S_CURLY_CLOSE:
        if (curlyBracketsCount_)
          curlyBracketsCount_--;
        break;
      case SignType::S_SQ_OPEN:
        if (squareBracketsCount_ < maxBraceDepth_)
          squareBracketsCount_++;
        else
          diagnoseOverflow(DiagID::square_bracket_overflow);
        break;
      case SignType::S_SQ_CLOSE:
        if (squareBracketsCount_)
          squareBracketsCount_--;
        break;
      case SignType::S_ROUND_OPEN:
        if (roundBracketsCount_ < maxBraceDepth_)
          roundBracketsCount_++;
        else 
          diagnoseOverflow(DiagID::round_bracket_overflow);
        break;
      case SignType::S_ROUND_CLOSE:
        if (roundBracketsCount_) 
          roundBracketsCount_--;
        break;
      default:
        fox_unreachable("unknown bracket type");
    }
    next();
    assert((tok.getSourceRange().getRawOffset() == 0) 
      && "Token is a sign but it's SourceRange offset is greater than zero?");
    return tok.getSourceRange().getBeginLoc();
  }
  return {};
}

SourceRange Parser::consumeKeyword(KeywordType k) {
  auto tok = getCurtok();
  if (tok.is(k)) {
    next();
    return tok.getSourceRange();
  }
  return {};
}

void Parser::consumeAny() {
  Token tok = getCurtok();
  // If it's a bracket, dispatch to consumeBracket. Else, just skip it.
  if (tok.isSign() && isBracket(tok.getSignType()))
    consumeBracket(tok.getSignType());
  else
    next();
}

void Parser::revertConsume() {
  undo();
  Token tok = getCurtok();
  // Lambda to diagnose an overflow and kill the parser.
  auto diagnoseOverflow = [&](DiagID id) {
    diagEngine.report(id, tok.getSourceRange());
    die();
  };
  if (isBracket(tok.getSignType())) {
    // Update bracket counters
    // We will be doing the exact opposite of what consumeBracket does !
    // That means : Decrementing the counter if a ( { [ is found, 
		// and incrementing it if a } ] ) is found.
    switch (tok.getSignType()) {
      case SignType::S_CURLY_OPEN:
        if (curlyBracketsCount_)
          curlyBracketsCount_--;
        break;
      case SignType::S_CURLY_CLOSE:
        if (curlyBracketsCount_ < maxBraceDepth_)
          curlyBracketsCount_++;
        else
          diagnoseOverflow(DiagID::curly_bracket_overflow);
        break;
      case SignType::S_SQ_OPEN:
        if (squareBracketsCount_)
          squareBracketsCount_--;
        break;
      case SignType::S_SQ_CLOSE:
        if (squareBracketsCount_ < maxBraceDepth_)
          squareBracketsCount_++;
        else
          diagnoseOverflow(DiagID::square_bracket_overflow);
        break;
      case SignType::S_ROUND_OPEN:
        if (roundBracketsCount_)
          roundBracketsCount_--;
        break;
      case SignType::S_ROUND_CLOSE:
        if (roundBracketsCount_ < maxBraceDepth_)
          roundBracketsCount_++;
        else
          diagnoseOverflow(DiagID::round_bracket_overflow);
        break;
      default:
        fox_unreachable("unknown bracket type");
    }
  }
}

void Parser::next() {
  if (tokenIterator_ != tokens.end())
    tokenIterator_++;
}

void Parser::undo() {
  if (tokenIterator_ != tokens.begin())
    tokenIterator_--;
}

bool Parser::isBracket(SignType s) const {
  switch (s) {
    case SignType::S_CURLY_OPEN:
    case SignType::S_CURLY_CLOSE:
    case SignType::S_SQ_OPEN:
    case SignType::S_SQ_CLOSE:
    case SignType::S_ROUND_OPEN:
    case SignType::S_ROUND_CLOSE:
      return true;
    default:
      return false;
  }
}

Parser::Result<TypeLoc> Parser::parseBuiltinTypename() {
  // <builtin_type_name>   = "int" | "float" | "bool" | "string" | "char"
  using RtrTy = Result<TypeLoc>;

  // "int"
  if (auto range = consumeKeyword(KeywordType::KW_INT))
    return RtrTy(TypeLoc(PrimitiveType::getInt(ctxt), range));
  
  // "float"
  if (auto range = consumeKeyword(KeywordType::KW_DOUBLE))
    return RtrTy(TypeLoc(PrimitiveType::getDouble(ctxt), range));

  // "bool"
  if (auto range = consumeKeyword(KeywordType::KW_BOOL))
    return RtrTy(TypeLoc(PrimitiveType::getBool(ctxt), range));

  // "string"
  if (auto range = consumeKeyword(KeywordType::KW_STRING))
    return RtrTy(TypeLoc(PrimitiveType::getString(ctxt), range));

  // "char"
  if (auto range = consumeKeyword(KeywordType::KW_CHAR))
    return RtrTy(TypeLoc(PrimitiveType::getChar(ctxt), range));

  return RtrTy::NotFound();
}

Parser::Result<TypeLoc> Parser::parseType() {
  // <type> =  ('[' <type> ']') | <builtin_type_name>
  // ('[' <type> ']')
  if(auto lSQBLoc = consumeBracket(SignType::S_SQ_OPEN)) {
    auto ty_res = parseType();
    bool error = !ty_res;
    if(error && ty_res.isNotFound())
      reportErrorExpected(DiagID::expected_type);
    auto rSQBLoc = consumeBracket(SignType::S_SQ_CLOSE);
    if(!rSQBLoc) {
      error = true;
      reportErrorExpected(DiagID::expected_closing_square_bracket);
      bool resync = resyncToSign(SignType::S_SQ_CLOSE,
        /*stop@semi*/ true, /*consume*/ false);
      if(resync)
        rSQBLoc = consumeBracket(SignType::S_SQ_CLOSE);
    }
    if(error)
      return Result<TypeLoc>::Error();
    
    SourceRange range(lSQBLoc, rSQBLoc);
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
  if (it != tokens.begin())
    return *(--it);
  return Token();
}

bool 
Parser::resyncToSign(SignType sign, bool stopAtSemi, bool shouldConsumeToken) {
  return resyncToSign(SmallVector<SignType, 4>({ sign }),
    stopAtSemi, shouldConsumeToken);
}

bool Parser::resyncToSign(const SmallVector<SignType, 4>& signs,
	bool stopAtSemi, bool shouldConsumeToken) {
  if (!isAlive())
    return false;

  bool isFirst = true;
  // Keep going until we reach EOF.
  while(!isDone()) {
    // Check the current token
    auto tok = getCurtok();
    for (const auto& sign : signs) {
      if (tok.is(sign)) {
        if (shouldConsumeToken)
          consumeAny();
        return true;
      }
    }

    if (tok.isSign()) {
      switch (tok.getSignType()) {
        // Skip nested brackets
        case SignType::S_CURLY_OPEN:
          consumeBracket(SignType::S_CURLY_OPEN);
          resyncToSign(SignType::S_CURLY_CLOSE, false, true);
          break;
        case SignType::S_SQ_OPEN:
          consumeBracket(SignType::S_SQ_OPEN);
          resyncToSign(SignType::S_SQ_CLOSE, false, true);
          break;
        case SignType::S_ROUND_OPEN:
          consumeBracket(SignType::S_ROUND_OPEN);
          resyncToSign(SignType::S_ROUND_CLOSE, false, true);
          break;
        // Skip closing brackets if they're unbalanced, else
        // return to avoid escaping the current block.
        case SignType::S_CURLY_CLOSE:
          if (curlyBracketsCount_ && !isFirst)
            return false;
          consumeBracket(SignType::S_CURLY_CLOSE);
          break;
        case SignType::S_SQ_CLOSE:
          if (squareBracketsCount_ && !isFirst)
            return false;
          consumeBracket(SignType::S_SQ_CLOSE);
          break;
        case SignType::S_ROUND_CLOSE:
          if (roundBracketsCount_ && !isFirst)
            return false;
          consumeBracket(SignType::S_ROUND_CLOSE);
          break;
        case SignType::S_SEMICOLON:
          if (stopAtSemi)
            return false;
          // fallthrough
        default:
          consumeAny();
          break;
      }
    } // (tok.isSign())
    else 
      consumeAny();

    isFirst = false;
  }
  // If reached eof, die & return false.
  die();
  return false;
}

bool Parser::resyncToNextDecl() {
  // This method skips everything until it finds a "let" or a "func".

  // Return immediately if recovery is not allowed, 
	// or the parser isn't alive anymore.
  if (!isAlive())
    return false;

  while(!isDone()) {
    auto tok = getCurtok();
    // if it's let/func, return.
    if (tok.is(KeywordType::KW_FUNC) || tok.is(KeywordType::KW_LET))
      return true;
    // Check if it's a sign for special behaviours
    if (tok.isSign()) {
      switch (tok.getSignType()) {
          // If we find a '(', '{' or '[', 
					// call resyncToSign to skip to it's counterpart.
        case SignType::S_CURLY_OPEN:
          consumeBracket(SignType::S_CURLY_OPEN);
          resyncToSign(SignType::S_CURLY_CLOSE, false, true);
          break;
        case SignType::S_SQ_OPEN:
          consumeBracket(SignType::S_SQ_OPEN);
          resyncToSign(SignType::S_SQ_CLOSE, false, true);
          break;
        case SignType::S_ROUND_OPEN:
          consumeBracket(SignType::S_ROUND_OPEN);
          resyncToSign(SignType::S_ROUND_CLOSE, false, true);
          break;
          // If we find a ')', '}' or ']' we just consume it.
        case SignType::S_CURLY_CLOSE:
          consumeBracket(SignType::S_CURLY_CLOSE);
          break;
        case SignType::S_SQ_CLOSE:
          consumeBracket(SignType::S_SQ_CLOSE);
          break;
        case SignType::S_ROUND_CLOSE:
          consumeBracket(SignType::S_ROUND_CLOSE);
          break;
        default:
          consumeAny();
          break;
      }
    } // (tok.isSign())
    else 
      consumeAny();
  }
  // If reached eof, die & return false.
  die();
  return false;
}

void Parser::die() {
  tokenIterator_ = tokens.end();
  isAlive_ = false;
}

Diagnostic Parser::reportErrorExpected(DiagID diag) {
  SourceRange errorRange;
  if (Token prevTok = getPreviousToken()) {
    SourceLoc loc = prevTok.getSourceRange().getEndLoc();
    // Get the next character in the file. This will be our 
    // error's location.
    loc = srcMgr.incrementSourceLoc(loc);
    errorRange = SourceRange(loc);
  }
  else {
    // No valid undo token, use the current token's range as the 
    // error location. (This case should be fairly rare, 
		// or never happen at all. tests needed)
    Token curTok = getCurtok();
    assert(curTok && "No valid previous token and no valid current token?");
    errorRange = curTok.getSourceRange();
  }
  return diagEngine.report(diag, errorRange);
}

bool Parser::isDone() const {
  return (tokenIterator_ == tokens.end()) || (!isAlive());
}

bool Parser::isAlive() const {
  return isAlive_;
}

DeclContext* Parser::getCurrentDeclCtxt() const {
  return curDeclCtxt_;
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