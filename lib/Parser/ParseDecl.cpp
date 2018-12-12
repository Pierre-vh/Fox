//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : ParseDecl.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file implements decl, declstmt rules (methods)
// and related helper functions
//----------------------------------------------------------------------------//

#include "Fox/Parser/Parser.hpp"
#include "Fox/AST/ASTContext.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"

using namespace fox;

UnitDecl* Parser::parseUnit(FileID fid, Identifier unitName, bool isMainUnit) {
  // <fox_unit>  = {<declaration>}1+

  // Assert that unitName != nullptr
  assert(unitName && "Unit name cannot be nullptr!");
  assert(fid && "FileID cannot be invalid!");

  // Create the unit
  assert(isDeclParentADeclCtxtOrNull() && "UnitDecls cannot be local decls!");
  auto* dc = getDeclParent().dyn_cast<DeclContext*>();
  auto* unit = UnitDecl::create(ctxt, dc, unitName, fid);

  // Create a RAIIDeclParent
  RAIIDeclParent raiidr(this, unit);

  bool declHadError = false;

  // Parse declarations 
  while (true) {
    if (auto parsedDecl = parseDecl()) {
      // We don't need to do anything, the Decl has been automatically
      // recorded because our DeclContext is the one that's currently active.
      continue;
    }
    else {
      if (!parsedDecl.wasSuccessful())
        declHadError = true;

      // EOF/Died -> Break.
      if (isDone())
        break;
      // No EOF? There's an unexpected token on the way that 
			// prevents us from finding the decl.
      else {
        // Report an error in case of "not found";
        if (parsedDecl.wasSuccessful()) {
          // Report the error with the current token being the error location
          Token curtok = getCurtok();
          assert(curtok 
            && "Curtok must be valid since we have not reached eof");
          diags.report(DiagID::parser_expected_decl, curtok.getRange());
        }

        if (resyncToNextDecl())
          continue;
        else
          break;
      }
    }

  }

  if (unit->numDecls() == 0) {
    if(!declHadError)
      diags.report(DiagID::parser_expected_decl_in_unit, fid);
    return nullptr;
  }
  else {
    ctxt.addUnit(unit, isMainUnit);
    return unit;
  }
}

Parser::DeclResult Parser::parseFuncDecl() {
  /*
    <func_decl>  = "func" <id> '(' [<param_decl> {',' <param_decl>}*] ')
									 '[':' <type>] <compound_statement>
    // Note about [':' <type>], if it isn't present, the function returns void
  */

  // FIXME:
    // 1) Improve the error recovery on a missing '(' or ')' 
    // 2) Split this method in multiples methods (e.g. parseFunctionParams)
    //    to improve readability.

  // "func"
  auto fnKw = consumeKeyword(KeywordType::KW_FUNC);
  if (!fnKw)
    return DeclResult::NotFound();

  // For FuncDecl, the return node is created prematurely as an "empty shell",
  // because we need it's DeclContext to exist to successfully record 
  // (inside it's DeclContext) it's ParamDecls and other decls that will
  // be parsed in it's body.
  auto* parentDC = getDeclParentAsDeclCtxt();
  FuncDecl* rtr = FuncDecl::create(ctxt, parentDC, Identifier(), 
    TypeLoc(), SourceRange(), SourceLoc());
  
  RAIIDeclParent parentGuard(this, rtr);

  // Useful location informations
  SourceLoc begLoc = fnKw.getBegin();
  SourceLoc headEndLoc;
  
  // Poisoned is set to true, it means that the declarations is missing
  // critical information to be considered valid. If that's the case,
  // we won't finish this declaration and we'll just return an error after
  // emitting all of our diagnostics.
  bool poisoned = false;

  // <id>
  if (auto foundID = consumeIdentifier()) 
    rtr->setIdentifier(foundID.get());
  else {
    reportErrorExpected(DiagID::parser_expected_iden);
    poisoned = true;
  }

  // '('
  if (!consumeBracket(SignType::S_ROUND_OPEN)) {
    if (poisoned) return DeclResult::Error();
    reportErrorExpected(DiagID::parser_expected_opening_roundbracket);
    return DeclResult::Error();
  }

  // [<param_decl> {',' <param_decl>}*]
  if (auto first = parseParamDecl()) {
    rtr->addParam(first.getAs<ParamDecl>());
    while (true) {
      if (consumeSign(SignType::S_COMMA)) {
        if (auto param = parseParamDecl())
          rtr->addParam(param.getAs<ParamDecl>());
        else {
          // IDEA: Maybe reporting the error after the "," would yield
          // better error messages?
          if (param.wasSuccessful()) 
            reportErrorExpected(DiagID::parser_expected_paramdecl);
          return DeclResult::Error();
        }
      } else break;
    }
  } 
  // Stop parsing if the argument couldn't parse correctly.
  else if (!first.wasSuccessful()) return DeclResult::Error();

  // ')'
  if (auto rightParens = consumeBracket(SignType::S_ROUND_CLOSE))
    headEndLoc = rightParens;
  else  {
    reportErrorExpected(DiagID::parser_expected_closing_roundbracket);

    // We'll attempt to recover to the '{' too,
		// so if we find the body of the function
    // we can at least parse that.
    if (!resyncToSign(SignType::S_ROUND_CLOSE, /*stop@semi*/ true, 
      /*consume*/ false))
      return DeclResult::Error();

    headEndLoc = consumeBracket(SignType::S_ROUND_CLOSE);
  }
  
  // [':' <type>]
  if (auto colon = consumeSign(SignType::S_COLON)) {
    if (auto rtrTy = parseType()) {
      TypeLoc tl = rtrTy.createTypeLoc();
      rtr->setReturnTypeLoc(tl);
      headEndLoc = tl.getRange().getEnd();
    }
    else {
      if (rtrTy.wasSuccessful())
        reportErrorExpected(DiagID::parser_expected_type);

      if (!resyncToSign(SignType::S_CURLY_OPEN, true, false))
        return DeclResult::Error();
      // If resynced successfully, use the colon as the end of the header
      // and consider the return type to be void
      headEndLoc = colon;
      rtr->setReturnTypeLoc(PrimitiveType::getVoid(ctxt));
    }
  }
  // if no return type, the function returns void.
  else rtr->setReturnTypeLoc(PrimitiveType::getVoid(ctxt));

  // <compound_statement>
  StmtResult compStmt = parseCompoundStatement();

  if (!compStmt) {
    if(compStmt.wasSuccessful()) // Display only if it was not found
      reportErrorExpected(DiagID::parser_expected_opening_curlybracket);
    return DeclResult::Error();
  }

  CompoundStmt* body = dyn_cast<CompoundStmt>(compStmt.get());
  assert(body && "Not a compound stmt");

  // Finished parsing. If the decl is poisoned, return an error.
  if (poisoned) return DeclResult::Error();

  parentGuard.restore();

  SourceRange range(begLoc, body->getRange().getEnd());
  assert(headEndLoc && range && "Invalid loc info");

  // Finish building our FuncDecl.
  rtr->setBody(body);
  rtr->setLocs(range, headEndLoc);
  recordInDeclCtxt(rtr);
  return DeclResult(rtr);
}

Parser::DeclResult Parser::parseParamDecl() {
  // <param_decl> = <id> ':' <qualtype>
  assert(isParsingFuncDecl() && "Can only call this when parsing a function!");
  // <id>
  auto id = consumeIdentifier();
  if (!id)
    return DeclResult::NotFound();

  // ':'
  if (!consumeSign(SignType::S_COLON)) {
    reportErrorExpected(DiagID::parser_expected_colon);
    return DeclResult::Error();
  }

  // <qualtype>
  auto typeResult = parseQualType();
  if (!typeResult) {
    if (typeResult.wasSuccessful())
      reportErrorExpected(DiagID::parser_expected_type);
    return DeclResult::Error();
  }

  TypeLoc tl(typeResult.get().type, typeResult.getRange());
  bool isConst = typeResult.get().isConst;

  SourceLoc begLoc = id.getRange().getBegin();
  SourceLoc endLoc = tl.getRange().getEnd();

  SourceRange range(begLoc, endLoc);

  assert(range && "Invalid loc info");

  auto* rtr = ParamDecl::create(ctxt, getDeclParent().get<FuncDecl*>(), 
    id.get(), tl, isConst, range);

  return DeclResult(rtr);
}

Parser::DeclResult Parser::parseVarDecl() {
  // <var_decl> = "let" <id> ':' <qualtype> ['=' <expr>] ';'
  // "let"
  auto letKw = consumeKeyword(KeywordType::KW_LET);
  if (!letKw)
    return DeclResult::NotFound();
  
  SourceLoc begLoc = letKw.getBegin();
  SourceLoc endLoc;

  Identifier id;
  TypeLoc type;
  bool isConst = false;
  Expr* iExpr = nullptr;

  // <id>
  if (auto foundID = consumeIdentifier())
    id = foundID.get();
  else {
    reportErrorExpected(DiagID::parser_expected_iden);
    if (auto res = resyncToSign(SignType::S_SEMICOLON, 
			/* stopAtSemi (true/false doesn't matter when we're looking for a semi) */ 
			false, /*consumeToken*/ true)) {
      // Recovered? Act like nothing happened.
      return DeclResult::NotFound();
    }
    return DeclResult::Error();
  }

  // ':'
  if (!consumeSign(SignType::S_COLON)) {
    reportErrorExpected(DiagID::parser_expected_colon);
    return DeclResult::Error();
  }

  // <qualtype>
  SourceLoc ampLoc;
  if (auto qtRes = parseQualType(nullptr, &ampLoc)) {
    type = TypeLoc(qtRes.get().type, qtRes.getRange());
    isConst = qtRes.get().isConst;
    if (qtRes.get().isRef)
      diags.report(DiagID::parser_ignored_ref_vardecl, ampLoc);
  }
  else {
    if (qtRes.wasSuccessful())
      reportErrorExpected(DiagID::parser_expected_type);
    if (auto res = resyncToSign(SignType::S_SEMICOLON, 
			/*stopAtSemi*/ true, /*consumeToken*/ true))
      return DeclResult::NotFound(); // Recovered? Act like nothing happened.
    return DeclResult::Error();
  }

  // ['=' <expr>]
  if (consumeSign(SignType::S_EQUAL)) {
    if (auto expr = parseExpr())
      iExpr = expr.get();
    else {
      if (expr.wasSuccessful())
        reportErrorExpected(DiagID::parser_expected_expr);
      // Recover to semicolon, return if recovery wasn't successful 
      if (!resyncToSign(SignType::S_SEMICOLON, 
				/*stopAtSemi*/ false, /*consumeToken*/ false))
        return DeclResult::Error();
    }
  }

  // ';'
  endLoc = consumeSign(SignType::S_SEMICOLON);
  if (!endLoc) {
    reportErrorExpected(DiagID::parser_expected_semi);
      
    if (!resyncToSign(SignType::S_SEMICOLON, 
			/*stopAtSemi*/ false, /*consumeToken*/ false))
      return DeclResult::Error();

    endLoc = consumeSign(SignType::S_SEMICOLON);
  }

  SourceRange range(begLoc, endLoc);
  assert(range && "Invalid loc info");
  assert(type && "type is not valid");
  assert(type.getRange() && "type range is not valid");
  auto rtr = VarDecl::create(ctxt, getDeclParent(),id, type, 
    isConst, iExpr, range);

  recordInDeclCtxt(rtr);
  return DeclResult(rtr);
}

Parser::Result<Parser::ParsedQualType> 
Parser::parseQualType(SourceRange* constRange, SourceLoc* refLoc) {
  //   <qualtype>  = ["const"] ['&'] <type>
  ParsedQualType rtr;
  bool hasFoundSomething = false;
  SourceLoc begLoc, endLoc;

  // ["const"]
  if (auto kw = consumeKeyword(KeywordType::KW_CONST)) {
    begLoc = kw.getBegin();
    hasFoundSomething = true;
    rtr.isConst = true;

    if (constRange)
      (*constRange) = kw;
  }

  // ['&']
  if (auto ampersand = consumeSign(SignType::S_AMPERSAND)) {
    // If no begLoc, the begLoc is the ampersand.
    if (!begLoc)
      begLoc = ampersand;
    hasFoundSomething = true;
    rtr.isRef = true;

    if (refLoc)
      (*refLoc) = ampersand;
  }

  // <type>
  if (auto tyRes = parseType()) {
    rtr.type = tyRes.get();

    // If no begLoc, the begLoc is the type's begLoc.
    if (!begLoc)
      begLoc = tyRes.getRange().getBegin();

    endLoc = tyRes.getRange().getEnd();
  }
  else {
    if (hasFoundSomething) {
      if (tyRes.wasSuccessful())
        reportErrorExpected(DiagID::parser_expected_type);
      return Result<ParsedQualType>::Error();
    }
    else 
      return Result<ParsedQualType>::NotFound();
  }

  assert(rtr.type && "Type cannot be invalid");
  assert(begLoc && "begLoc must be valid");
  assert(endLoc && "endLoc must be valid");
  return Result<ParsedQualType>(rtr, SourceRange(begLoc,endLoc));
}

Parser::DeclResult Parser::parseDecl() {
  // <declaration> = <var_decl> | <func_decl>

  // <var_decl>
  if (auto vdecl = parseVarDecl())
    return vdecl;
  else if (!vdecl.wasSuccessful())
    return DeclResult::Error();

  // <func_decl>
  if (auto fdecl = parseFuncDecl())
    return fdecl;
  else if (!fdecl.wasSuccessful())
    return DeclResult::Error();

  return DeclResult::NotFound();
}