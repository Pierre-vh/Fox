//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : ParseDecl.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Parser/Parser.hpp"
#include "Fox/AST/ASTContext.hpp"
#include "Fox/AST/Types.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"

using namespace fox;

void Parser::finishDecl(Decl* decl) {
  assert(decl && "The Declaration is null!");
  // If we have a currently active DDR, add it to the DDR, else
  // register the decl directly.
  curDDR_ ? curDDR_->addDecl(decl) : registerDecl(decl, ScopeInfo());
}

void Parser::registerDecl(Decl* decl, ScopeInfo scopeInfo) {
  // Record the decl in its DeclContext
  if (DeclContext* dc = decl->getDeclContext()) {
    assert(decl->isLocal() || (!decl->isLocal() && !scopeInfo) &&
      "Non-null ScopeInfo is only supported for local decls!");
    dc->addDecl(decl, scopeInfo);
  }
  // Only UnitDecls shouldn't have parents.
  else 
    assert(isa<UnitDecl>(decl) && "Decl doesn't have a DeclContext "
      "and it isn't a UnitDecl?");
}

UnitDecl* Parser::parseUnit(Identifier unitName) {
  // <fox_unit>  = {<declaration>}1+
  FileID file = getFileID();
  assert(unitName && "Unit name cannot be nullptr!");
  assert(file && "FileID cannot be invalid!");

  // Create the unit
  auto* unit = UnitDecl::create(ctxt, unitName, file);

  // Create a RAIIDeclCtxt
  RAIIDeclCtxt raiidr(this, unit);

  bool declHadError = false;

  // Parse declarations 
  while (true) {
    if (auto parsedDecl = parseDecl()) {
      // We don't need to do anything, the Decl has been automatically
      // recorded because our DeclContext is the one that's currently active.
      continue;
    }
    else {
      if (parsedDecl.isError()) declHadError = true;

      // EOF -> Break.
      if (isDone()) break;
      // No EOF? There's an unexpected token on the way that 
			// prevents us from finding the decl, so try to recover.
      if (skipUntilDecl()) 
        continue; 
      else 
        break;
    }
  }

  if (unit->getDecls().isEmpty()) {
    if(!declHadError)
      diagEngine.report(DiagID::expected_decl_in_unit, file);
    return nullptr;
  }
  else {
    finishDecl(unit);
    return unit;
  }
}

Parser::Result<Decl*> Parser::parseFuncDecl() {
  /*
    <func_decl>  = "func" <id> '(' [<param_decl> {',' <param_decl>}*] ')
									 '[':' <type>] <compound_stmt>
    // Note about [':' <type>], if it isn't present, the function returns void
  */

  // "func"
  auto fnKw = tryConsume(TokenKind::FuncKw);
  if (!fnKw) return Result<Decl*>::NotFound();
  assert(fnKw.getBeginLoc() && "invalid loc info for func token");


  // Location information
  SourceLoc begLoc = fnKw.getBeginLoc();

  // <id>
  Identifier id;
  SourceRange idRange;
  if (isCurTokAnIdentifier())
    std::tie(id, idRange) = consumeIdentifier();
  else {
    reportErrorExpected(DiagID::expected_iden);
    return Result<Decl*>::Error();
  }

  // Once we know the Identifier, we can create the FuncDecl instance.
  // That instance will be completed later, or simply discarded if
  // parsing errors occur.
  FuncDecl* func = FuncDecl::create(ctxt, getCurrentDeclCtxt(), begLoc,
  id, idRange, nullptr, TypeLoc());

  // Enter this func's DeclContext
  RAIIDeclCtxt raiiDC(this, func);

  // When parsing a FuncDecl, we don't return immediatly on error, instead
  // we try hard to parse the whole function (or at least its body).
  bool hadError = false;

  // '('
  SourceLoc leftParen = tryConsume(TokenKind::LParen).getBeginLoc();
  if (!leftParen) {
    reportErrorExpected(DiagID::expected_lparen);
    hadError = true;
  }

  // [<param_decl> {',' <param_decl>}*]
  {
    bool paramHadError = false;
    // try to parse the first argument
    if (auto first = parseParamDecl()) {
      SmallVector<ParamDecl*, 4> paramsVec;
      paramsVec.push_back(first.castTo<ParamDecl>());
      while (true) {
        if (tryConsume(TokenKind::Comma)) {
          if (auto param = parseParamDecl())
            paramsVec.push_back(param.castTo<ParamDecl>());
          else {
            paramHadError = true;
            if (param.isNotFound())
              reportErrorExpected(DiagID::expected_paramdecl);
            break;
          }
        } 
        else
          break;
      }
      func->setParams(ParamList::create(ctxt, paramsVec));
    }
    else 
      paramHadError = first.isError();

    // Try to recover if any param had an error.
    if (paramHadError) {
      if(!skipUntilDeclStmtOr(TokenKind::LBrace))
        return Result<Decl*>::Error();
      hadError = true;
    }
  }

  // ')'
  auto rightParens = tryConsume(TokenKind::RParen);
  // Diagnose if we had a '('
  if (!rightParens && leftParen && !hadError) {
    reportErrorExpected(DiagID::expected_rparen);
    diagEngine.report(DiagID::to_match_this_paren, leftParen);
    hadError = true;
  }
  
  // [':' <type>]
  if (auto colon = tryConsume(TokenKind::Colon)) {
    if (auto rtrTy = parseType())
      func->setReturnTypeLoc(rtrTy.get());
    else if (rtrTy.isNotFound() && !hadError) {
      reportErrorExpected(DiagID::expected_type);
      hadError = true;
    }
  } 
  else {
    // No explicit return type, so the function returns void.
    TypeLoc voidTL(VoidType::get(ctxt), SourceRange());
    func->setReturnTypeLoc(voidTL);
  }

  // <compound_stmt>
  if(Result<Stmt*> compStmt = parseCompoundStatement())
    func->setBody(cast<CompoundStmt>(compStmt.get()));
  else {
    if(compStmt.isNotFound() && !hadError)
      reportErrorExpected(DiagID::expected_lbrace);
    return Result<Decl*>::Error();
  }


  // Leave this func's scope, so we don't get into an infinite loop when calling
  // finishDecl.
  raiiDC.restore();

  if(hadError)
    return Result<Decl*>::Error();

  // Record the FuncDecl
  finishDecl(func);
  // Calculate it's ValueType.
  func->calculateValueType();
  assert(func->getValueType() && "FuncDecl type not calculated");
  return Result<Decl*>(func);
}

Parser::Result<Decl*> Parser::parseParamDecl() {
  // <param_decl> = <id> ':' ["mut"] <type>

  // <id>
  if (!isCurTokAnIdentifier())
    return Result<Decl*>::NotFound();

  Identifier id;
  SourceRange idRange;
  std::tie(id, idRange) = consumeIdentifier();

  // ':'
  if (!tryConsume(TokenKind::Colon)) {
    reportErrorExpected(DiagID::expected_colon);
    return Result<Decl*>::Error();
  }

  auto isMut = (bool)tryConsume(TokenKind::MutKw);

  // <type>
  auto typeResult = parseType();
  if (!typeResult) {
    if (typeResult.isNotFound())
      reportErrorExpected(DiagID::expected_type);
    return Result<Decl*>::Error();
  }

  TypeLoc tl = typeResult.get();

  assert(idRange && tl.getSourceRange() && "Invalid loc info");

  auto* rtr = ParamDecl::create(ctxt, getCurrentDeclCtxt(), id, idRange, 
                                tl, isMut); 
  finishDecl(rtr);
  return Result<Decl*>(rtr);
}

Parser::Result<Decl*> Parser::parseVarDecl() {
  // <var_decl> = ("let" | "var") <id> ':' <type> ['=' <expr>] ';'

  // ("let" | "var")
  VarDecl::Keyword kw;
  SourceLoc begLoc;
  if (auto letKw = tryConsume(TokenKind::LetKw)) {
    kw = VarDecl::Keyword::Let;
    begLoc = letKw.getBeginLoc();
  } 
  else if(auto varKw = tryConsume(TokenKind::VarKw)) {
    kw = VarDecl::Keyword::Var;
    begLoc = varKw.getBeginLoc();
  }
  else
    return Result<Decl*>::NotFound();
 

  // <id>
  if(!isCurTokAnIdentifier()) {
    reportErrorExpected(DiagID::expected_iden);
    return Result<Decl*>::Error();
  }

  Identifier id;
  SourceRange idRange;
  std::tie(id, idRange) = consumeIdentifier();

  // ':'
  if (!tryConsume(TokenKind::Colon)) {
    reportErrorExpected(DiagID::expected_colon);
    return Result<Decl*>::Error();
  }

  // <type>
  TypeLoc typeLoc;
  if (auto typeRes = parseType())
    typeLoc = typeRes.get();
  else {
    if (typeRes.isNotFound())
      reportErrorExpected(DiagID::expected_type);
    return Result<Decl*>::Error();
  }
  assert(typeLoc.isComplete() && "Incomplete TypeLoc!");

  // ['=' <expr>]
  Expr* initializer = nullptr;
  if (tryConsume(TokenKind::Equal)) {
    if (auto expr = parseExpr())
      initializer = expr.get();
    else {
      if (expr.isNotFound())
        reportErrorExpected(DiagID::expected_expr);
      return Result<Decl*>::Error();
    }
  }

  // ';'
  if (!tryConsume(TokenKind::Semi).getBeginLoc()) {
    reportErrorExpected(DiagID::expected_semi);
    return Result<Decl*>::Error();
  }

  // Deduce the endLoc: it's either the Expr's EndLoc or the
  // TypeLoc's.
  SourceLoc endLoc;
  if (initializer) {
    endLoc = initializer->getEndLoc();
    assert(endLoc && "initializer doesn't have an end loc?");
  }
  else {
    endLoc = typeLoc.getEndLoc();
    assert(endLoc && "TypeLoc doesn't have an end loc?");
  }

  SourceRange range(begLoc, endLoc);
  assert(range && idRange && "Invalid loc info");

  auto rtr = VarDecl::create(ctxt, getCurrentDeclCtxt(), id, idRange,
    typeLoc, kw, initializer, range);

  finishDecl(rtr);
  return Result<Decl*>(rtr);
}

Parser::Result<Decl*> Parser::parseDecl() {
  // <declaration> = <var_decl> | <func_decl>

  // <var_decl>
  if (auto vdecl = parseVarDecl())
    return vdecl;
  else if (vdecl.isError())
    return Result<Decl*>::Error();

  // <func_decl>
  if (auto fdecl = parseFuncDecl())
    return fdecl;
  else if (fdecl.isError())
    return Result<Decl*>::Error();

  return Result<Decl*>::NotFound();
}