//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
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
#include "Fox/AST/Types.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"

using namespace fox;

Parser::Parser(ASTContext& ctxt, TokenVector& l, UnitDecl *unit):
  ctxt(ctxt), tokens(l), srcMgr(ctxt.sourceMgr), diags(ctxt.diagEngine),
  curParent_(unit) {
  tokenIterator_ = tokens.begin();
  isAlive_ = true;
}

void Parser::actOnDecl(Decl* decl) {
  assert(decl && "decl is null!");

  if(decl->isParentNull()) {
    assert(isa<UnitDecl>(decl) && "Only UnitDecls are able to have a "
      "null parent!");
    return;
  }

  if(!decl->isLocal()) {
    // Fetch the DeclContext
    DeclContext* dc = decl->getDeclContext();
    assert(dc && "Parent isn't null but getDeclContext returns nullptr?");
    // Check that we can add it to the DeclContext safely
    if(NamedDecl* named = dyn_cast<NamedDecl>(decl)) {
      assert(named->hasIdentifier() 
        && "NamedDecl must have a valid Identifier!");
    }
    // Add it to the DeclContext
    dc->addDecl(decl);
  }
  else {
    assert(!isa<FuncDecl>(decl) && "FuncDecls cannot be local");
    assert(decl->getFuncDecl() && "Decl is local but doesn't have a non-null "
      "FuncDecl as Parent?");
  }
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
        if (curlyBracketsCount_ < maxBraceDepth_)
          curlyBracketsCount_++;
        else {
          diags.report(DiagID::parser_curlybracket_overflow);
          die();
        }
        break;
      case SignType::S_CURLY_CLOSE:
        if (curlyBracketsCount_)
          curlyBracketsCount_--;
        break;
      case SignType::S_SQ_OPEN:
        if (squareBracketsCount_ < maxBraceDepth_)
          squareBracketsCount_++;
        else {
          diags.report(DiagID::parser_squarebracket_overflow);
          die();
        }
        break;
      case SignType::S_SQ_CLOSE:
        if (squareBracketsCount_)
          squareBracketsCount_--;
        break;
      case SignType::S_ROUND_OPEN:
        if (roundBracketsCount_ < maxBraceDepth_)
          roundBracketsCount_++;
        else {
          diags.report(DiagID::parser_roundbracket_overflow);
          die();
        }
        break;
      case SignType::S_ROUND_CLOSE:
        if (roundBracketsCount_) 
          roundBracketsCount_--;
        break;
      default:
        fox_unreachable("unknown bracket type");
    }
    next();
    assert((tok.getRange().getRawOffset() == 0) 
      && "Token is a sign but it's SourceRange offset is greater than zero?");
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
        if (curlyBracketsCount_)
          curlyBracketsCount_--;
        break;
      case SignType::S_CURLY_CLOSE:
        if (curlyBracketsCount_ < maxBraceDepth_)
          curlyBracketsCount_++;
        else {
          diags.report(DiagID::parser_curlybracket_overflow);
          die();
        }
        break;
      case SignType::S_SQ_OPEN:
        if (squareBracketsCount_)
          squareBracketsCount_--;
        break;
      case SignType::S_SQ_CLOSE:
        if (squareBracketsCount_ < maxBraceDepth_)
          squareBracketsCount_++;
        else {
          diags.report(DiagID::parser_squarebracket_overflow);
          die();
        }
        break;
      case SignType::S_ROUND_OPEN:
        if (roundBracketsCount_)
          roundBracketsCount_--;
        break;
      case SignType::S_ROUND_CLOSE:
        if (roundBracketsCount_ < maxBraceDepth_)
          roundBracketsCount_++;
        else {
          diags.report(DiagID::parser_roundbracket_overflow);
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

Parser::Result<Type> Parser::parseBuiltinTypename() {
  // <builtin_type_name>   = "int" | "float" | "bool" | "string" | "char"
  using RtrTy = Result<Type>;

  // "int"
  if (auto range = consumeKeyword(KeywordType::KW_INT))
    return RtrTy(Type(PrimitiveType::getInt(ctxt)), range);
  
  // "float"
  if (auto range = consumeKeyword(KeywordType::KW_DOUBLE))
    return RtrTy(Type(PrimitiveType::getDouble(ctxt)), range);

  // "bool"
  if (auto range = consumeKeyword(KeywordType::KW_BOOL))
    return RtrTy(Type(PrimitiveType::getBool(ctxt)), range);

  // "string"
  if (auto range = consumeKeyword(KeywordType::KW_STRING))
    return RtrTy(Type(PrimitiveType::getString(ctxt)), range);

  // "char"
  if (auto range = consumeKeyword(KeywordType::KW_CHAR))
    return RtrTy(Type(PrimitiveType::getChar(ctxt)), range);

  return RtrTy::NotFound();
}

Parser::Result<Type> Parser::parseType() {
  // <type> =  ('[' <type> ']') | <builtin_type_name>
  // ('[' <type> ']')
  if(auto lSQBLoc = consumeBracket(SignType::S_SQ_OPEN)) {
    auto ty_res = parseType();
    bool error = !ty_res;
    if(error && ty_res.isNotFound())
      reportErrorExpected(DiagID::parser_expected_type);
    auto rSQBLoc = consumeBracket(SignType::S_SQ_CLOSE);
    if(!rSQBLoc) {
      error = true;
      reportErrorExpected(DiagID::parser_expected_closing_squarebracket);
      bool resync = resyncToSign(SignType::S_SQ_CLOSE,
        /*stop@semi*/ true, /*consume*/ false);
      if(resync)
        rSQBLoc = consumeBracket(SignType::S_SQ_CLOSE);
    }
    if(error)
      return Result<Type>::Error();
    
    SourceRange range(lSQBLoc, rSQBLoc);
    assert(range && "range should be valid");
    Type ty = ty_res.get();
    ty = ArrayType::get(ctxt, ty);
    return Result<Type>(ty, range);
  }
  // <builtin_type_name> 
  if (auto ty_res = parseBuiltinTypename()) 
    return ty_res;
  return Result<Type>::NotFound();
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
    SourceLoc loc = prevTok.getRange().getEnd();
    // Get the next character in the file. This will be our 
    // error's location.
    loc = srcMgr.getNextCodepointSourceLoc(loc);
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
  return diags.report(diag, errorRange);
}

bool Parser::isDone() const {
  return (tokenIterator_ == tokens.end()) || (!isAlive());
}

bool Parser::isAlive() const {
  return isAlive_;
}

bool Parser::isParsingFuncDecl() const {
  return curParent_.is<FuncDecl*>();
}

bool Parser::isDeclParentADeclCtxtOrNull() const {
  if(!curParent_.isNull())
    return curParent_.is<DeclContext*>();
  return true;
}

DeclContext* Parser::getDeclParentAsDeclCtxt() const {
  assert(isDeclParentADeclCtxtOrNull() && "DeclParent must "
    "be a DeclContext or nullptr!");
  return getDeclParent().dyn_cast<DeclContext*>();
}

// RAIIDeclParent
Parser::RAIIDeclParent::RAIIDeclParent(Parser *p, Decl::Parent parent):
  parser_(p) {
  assert(p && "Parser instance can't be nullptr");
  lastParent_ = p->curParent_;
  p->curParent_ = parent;
}

void Parser::RAIIDeclParent::restore() {
  assert(parser_ && "Parser instance can't be nullptr");
  parser_->curParent_ = lastParent_;
  parser_ = nullptr;
}

Parser::RAIIDeclParent::~RAIIDeclParent() {
  if(parser_) // parser_ will be nullptr if we restored early
    restore();
}