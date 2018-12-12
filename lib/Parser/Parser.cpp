//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : Parser.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  This file implements methods that aren't tied to Expression,
//  Statements or Declarations.
//----------------------------------------------------------------------------//

#include "Fox/Parser/Parser.hpp"
#include "Fox/AST/ASTContext.hpp"
#include "Fox/Common/Source.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"

using namespace fox;

Parser::Parser(DiagnosticEngine& diags, SourceManager &sm, ASTContext& astctxt, 
	TokenVector& l, DeclContext *declCtxt):
  ctxt_(astctxt), tokens_(l), srcMgr_(sm), diags_(diags) {
  if (declCtxt) state_.curParent = declCtxt;
  setupParser();
}

ASTContext& Parser::getASTContext() {
  return ctxt_;
}

SourceManager& Parser::getSourceManager() {
  return srcMgr_;
}

DiagnosticEngine& Parser::getDiagnosticEngine() {
  return diags_;
}

void Parser::setupParser() {
  state_.tokenIterator = tokens_.begin();
}

void Parser::recordInDeclCtxt(NamedDecl* decl) {
  auto parent = getDeclParent();
  if(auto* dc = parent.dyn_cast<DeclContext*>())
    dc->addDecl(decl);
}

Parser::Result<Identifier> Parser::consumeIdentifier() {
  Token tok = getCurtok();
  if (tok.isIdentifier()) {
    Identifier id = tok.getIdentifier();
    assert(id && "Token's an identifier but contains a null Identifier?");
    next();
    return Result<Identifier>(id, tok.getRange());
  }
  return Result<Identifier>::NotFound();
}

SourceLoc Parser::consumeSign(SignType s) {
  assert(!isBracket(s) 
		&& "This method shouldn't be used to match brackets ! "
			  "Use consumeBracket instead!");
  auto tok = getCurtok();
  if (tok.is(s)) {
    next();
    return tok.getRange().getBegin();
  }
  return SourceLoc();
}

SourceLoc Parser::consumeBracket(SignType s) {
  assert(isBracket(s) 
	&& "This method should only be used on brackets! "
	  	"Use consumeSign to match instead!");
  auto tok = getCurtok();
  if (tok.is(s)) {
    switch (s) {
      case SignType::S_CURLY_OPEN:
        if (state_.curlyBracketsCount < maxBraceDepth_)
          state_.curlyBracketsCount++;
        else {
          diags_.report(DiagID::parser_curlybracket_overflow);
          die();
        }
        break;
      case SignType::S_CURLY_CLOSE:
				// Don't let unbalanced parentheses underflow
        if (state_.curlyBracketsCount)
          state_.curlyBracketsCount--;
        break;
      case SignType::S_SQ_OPEN:
        if (state_.squareBracketsCount < maxBraceDepth_)
          state_.squareBracketsCount++;
        else {
          diags_.report(DiagID::parser_squarebracket_overflow);
          die();
        }
        break;
      case SignType::S_SQ_CLOSE:
				// Don't let unbalanced parentheses underflow
        if (state_.squareBracketsCount)
          state_.squareBracketsCount--;
        break;
      case SignType::S_ROUND_OPEN:
        if (state_.roundBracketsCount < maxBraceDepth_)
          state_.roundBracketsCount++;
        else {
          diags_.report(DiagID::parser_roundbracket_overflow);
          die();
        }
        break;
      case SignType::S_ROUND_CLOSE:
        if (state_.roundBracketsCount)    // Don't let unbalanced parentheses create an underflow.
          state_.roundBracketsCount--;
        break;
      default:
        fox_unreachable("unknown bracket type");
    }
    next();
    assert((tok.getRange().getOffset() == 0) && "Token is a sign but it's SourceRange offset is greater than zero?");
    return SourceLoc(tok.getRange().getBegin());
  }
  return SourceLoc();
}

SourceRange Parser::consumeKeyword(KeywordType k) {
  auto tok = getCurtok();
  if (tok.is(k)) {
    next();
    return tok.getRange();
  }
  return SourceRange();
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

  if (isBracket(tok.getSignType())) {
    // Update bracket counters
    // We will be doing the exact opposite of what consumeBracket does !
    // That means : Decrementing the counter if a ( { [ is found, 
		// and incrementing it if a } ] ) is found.
    switch (tok.getSignType()) {
      case SignType::S_CURLY_OPEN:
        if (state_.curlyBracketsCount)
          state_.curlyBracketsCount--;
        break;
      case SignType::S_CURLY_CLOSE:
        if (state_.curlyBracketsCount < maxBraceDepth_)
          state_.curlyBracketsCount++;
        else {
          diags_.report(DiagID::parser_curlybracket_overflow);
          die();
        }
        break;
      case SignType::S_SQ_OPEN:
        if (state_.squareBracketsCount)
          state_.squareBracketsCount--;
        break;
      case SignType::S_SQ_CLOSE:
        if (state_.squareBracketsCount < maxBraceDepth_)
          state_.squareBracketsCount++;
        else {
          diags_.report(DiagID::parser_squarebracket_overflow);
          die();
        }
        break;
      case SignType::S_ROUND_OPEN:
        if (state_.roundBracketsCount)
          state_.roundBracketsCount--;
        break;
      case SignType::S_ROUND_CLOSE:
        if (state_.roundBracketsCount < maxBraceDepth_)
          state_.roundBracketsCount++;
        else {
          diags_.report(DiagID::parser_roundbracket_overflow);
          die();
        }
        break;
      default:
        fox_unreachable("unknown bracket type");
    }
  }
  // Else, we're done. For now, only brackets have counters 
	// associated with them.
}

void Parser::next() {
  if (state_.tokenIterator != tokens_.end())
    state_.tokenIterator++;
}

void Parser::undo() {
  if (state_.tokenIterator != tokens_.begin())
    state_.tokenIterator--;
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

Parser::Result<Type> Parser::parseBuiltinTypename() {
  // <builtin_type_name>   = "int" | "float" | "bool" | "string" | "char"
  using RtrTy = Result<Type>;

  // "int"
  if (auto range = consumeKeyword(KeywordType::KW_INT))
    return RtrTy(Type(PrimitiveType::getInt(ctxt_)), range);
  
  // "float"
  if (auto range = consumeKeyword(KeywordType::KW_FLOAT))
    return RtrTy(Type(PrimitiveType::getFloat(ctxt_)), range);

  // "bool"
  if (auto range = consumeKeyword(KeywordType::KW_BOOL))
    return RtrTy(Type(PrimitiveType::getBool(ctxt_)), range);

  // "string"
  if (auto range = consumeKeyword(KeywordType::KW_STRING))
    return RtrTy(Type(PrimitiveType::getString(ctxt_)), range);

  // "char"
  if (auto range = consumeKeyword(KeywordType::KW_CHAR))
    return RtrTy(Type(PrimitiveType::getChar(ctxt_)), range);

  return RtrTy::NotFound();
}

Parser::Result<Type> Parser::parseType() {
  // <type> = <builtin_type_name> { '[' ']' }
  // <builtin_type_name> 
  if (auto ty_res = parseBuiltinTypename()) {
    //  { '[' ']' }
    Type ty = ty_res.get();
    SourceLoc begLoc = ty_res.getRange().getBegin();
    SourceLoc endLoc = ty_res.getRange().getEnd();

    while (consumeBracket(SignType::S_SQ_OPEN)) {
      ty = ArrayType::get(ctxt_, ty.getPtr());
      // ']'
      if (auto right = consumeBracket(SignType::S_SQ_CLOSE))
        endLoc = right;
      else {
        reportErrorExpected(DiagID::parser_expected_closing_squarebracket);

        if (resyncToSign(SignType::S_SQ_CLOSE,
					/*stop@semi */ true, /*consume*/ false)) {
          endLoc = consumeBracket(SignType::S_SQ_CLOSE);
          continue;
        }

        return Result<Type>::Error();
      }
    }

    // rebuild the TypeLoc with the updated loc info.
    return Result<Type>(ty, SourceRange(begLoc, endLoc));
  }
  return Result<Type>::NotFound();
}

Token Parser::getCurtok() const {
  if (!isDone())
    return *state_.tokenIterator;
  return Token();
}

Token Parser::getPreviousToken() const {
  auto it = state_.tokenIterator;
  if (it != tokens_.begin())
    return *(--it);
  return Token();
}

bool 
Parser::resyncToSign(SignType sign, bool stopAtSemi, bool shouldConsumeToken) {
  return resyncToSign(
		std::vector<SignType>({ sign }), stopAtSemi, shouldConsumeToken);
}

bool Parser::resyncToSign(const std::vector<SignType>& signs,
	bool stopAtSemi, bool shouldConsumeToken) {
  if (!isAlive())
    return false;

  bool isFirst = true;
  // Keep going until we reach EOF.
  while(!isDone()) {
    // Check curtok
    auto tok = getCurtok();
    for (auto it = signs.begin(), end = signs.end(); it != end; it++) {
      if (tok.is(*it)) {
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
          if (state_.curlyBracketsCount && !isFirst)
            return false;
          consumeBracket(SignType::S_CURLY_CLOSE);
          break;
        case SignType::S_SQ_CLOSE:
          if (state_.squareBracketsCount && !isFirst)
            return false;
          consumeBracket(SignType::S_SQ_CLOSE);
          break;
        case SignType::S_ROUND_CLOSE:
          if (state_.roundBracketsCount && !isFirst)
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
  state_.tokenIterator = tokens_.end();
  state_.isAlive = false;
}

Diagnostic Parser::reportErrorExpected(DiagID diag) {
  SourceRange errorRange;
  if (Token prevTok = getPreviousToken()) {
    SourceLoc loc = prevTok.getRange().getEnd();
    loc.increment();
    assert(srcMgr_.checkValid(loc));
    errorRange = SourceRange(loc);
  }
  else {
    // No valid undo token, use the current token's range as the 
    // error location. (This case should be fairly rare, 
		// or never happen at all. tests needed)
    Token curTok = getCurtok();
    assert(curTok && "No valid previous token and no valid current token?");
    errorRange = curTok.getRange();
  }
  return diags_.report(diag, errorRange);
}

bool Parser::isDone() const {
  return (state_.tokenIterator == tokens_.end()) || (!isAlive());
}

bool Parser::isAlive() const {
  return state_.isAlive;
}

bool Parser::isParsingFunction() const {
  return state_.curParent.is<FuncDecl*>();
}

bool Parser::isDeclParentADeclCtxtOrNull() const {
  if(!state_.curParent.isNull())
    return state_.curParent.is<DeclContext*>();
  return true;
}

DeclContext* Parser::getDeclParentAsDeclCtxt() const {
  assert(isDeclParentADeclCtxtOrNull() && "DeclParent must "
    "be a DeclContext or nullptr!");
  return getDeclParent().dyn_cast<DeclContext*>();
}

Parser::ParserState Parser::createParserStateBackup() const {
  return state_;
}

void Parser::restoreParserStateFromBackup(const Parser::ParserState & st) {
  state_ = st;
}

// ParserState
Parser::ParserState::ParserState():
  isAlive(true){

}

// RAIIDeclParent
Parser::RAIIDeclParent::RAIIDeclParent(Parser *p, Decl::Parent parent):
  parser_(p) {
  assert(p && "Parser instance can't be nullptr");
  lastParent_ = p->getDeclParent();

  // if "parent" is a DeclContext, and lastParent is too, set
  // "parent"'s parent to lastParent_
  auto* pDC = parent.dyn_cast<DeclContext*>();
  auto* lpDC = lastParent_.dyn_cast<DeclContext*>();
  if(pDC && lpDC)
    pDC->setParentDeclCtxt(lpDC);

  p->state_.curParent = parent;
}

void Parser::RAIIDeclParent::restore() {
  assert(parser_ && "Parser instance can't be nullptr");
  parser_->state_.curParent = lastParent_;
  parser_ = nullptr;
}

Parser::RAIIDeclParent::~RAIIDeclParent() {
  if(parser_) // parser_ will be nullptr if we restored early
    restore();
}