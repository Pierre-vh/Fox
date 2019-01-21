//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
// See LICENSE.txt for license info.            
// File : ParseDecl.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file implements decl, declstmt rules (methods)
// and related helper functions
//----------------------------------------------------------------------------//

#include "Fox/Parser/Parser.hpp"
#include "Fox/AST/ASTContext.hpp"
#include "Fox/AST/Types.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"

using namespace fox;

UnitDecl* Parser::parseUnit(FileID fid, Identifier unitName) {
  // <fox_unit>  = {<declaration>}1+

  // Assert that unitName != nullptr
  assert(unitName && "Unit name cannot be nullptr!");
  assert(fid && "FileID cannot be invalid!");

  // Create the unit
  auto* unit = UnitDecl::create(ctxt, unitName, fid);

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
      if (!parsedDecl.wasSuccessful()) declHadError = true;

      // EOF/Died -> Break.
      if (isDone()) break;

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

        if (resyncToNextDecl()) continue; 
        else break;
      }
    }
  }

  if (unit->numDecls() == 0) {
    if(!declHadError)
      diags.report(DiagID::parser_expected_decl_in_unit, fid);
    return nullptr;
  }
  else {
    ctxt.setUnit(unit);
    actOnDecl(unit);
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
  if (!fnKw) return DeclResult::NotFound();
  assert(fnKw.getBegin() && "invalid loc info for func token");
  // For FuncDecl, the return node is created prematurely as an "empty shell",
  // because we need it to exist so declarations that are parsed inside it's body
  // can be notified that they are being parsed as part of a declaration.
  //
  // Note that the FuncDecl's "shortened" create method automatically sets
  // the Return type to void.
  auto* parentDC = getDeclParentAsDeclCtxt();
  FuncDecl* rtr = FuncDecl::create(ctxt, parentDC, fnKw.getBegin());

  // Create a RAIIDeclParent to notify every parsing function that
  // we're currently parsing a FuncDecl
  RAIIDeclParent parentGuard(this, rtr);

  // Location information
  SourceLoc begLoc = fnKw.getBegin();
  
  // If invalid is set to true, it means that the declarations is missing
  // critical information and can't be considered as valid. If that's the case,
  // we won't return the declaration and we'll just return an error after
  // emitting all of our diagnostics.
  bool invalid = false;

  // <id>
  if (auto foundID = consumeIdentifier()) 
    rtr->setIdentifier(foundID.get(), foundID.getRange());
  else {
    reportErrorExpected(DiagID::parser_expected_iden);
    invalid = true;
  }

  // '('
  if (!consumeBracket(SignType::S_ROUND_OPEN)) {
    if (invalid) return DeclResult::Error();
    reportErrorExpected(DiagID::parser_expected_opening_roundbracket);
    return DeclResult::Error();
  }

  // [<param_decl> {',' <param_decl>}*]
  SmallVector<ParamDecl*, 4> params;
  if (auto first = parseParamDecl()) {
    params.push_back(first.getAs<ParamDecl>());
    while (true) {
      if (consumeSign(SignType::S_COMMA)) {
        if (auto param = parseParamDecl())
          params.push_back(param.getAs<ParamDecl>());
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
  auto rightParens = consumeBracket(SignType::S_ROUND_CLOSE);
  if (!rightParens) {
    reportErrorExpected(DiagID::parser_expected_closing_roundbracket);

    // We'll attempt to recover to the '{' too,
		// so if we find the body of the function
    // we can at least parse that.
    if (!resyncToSign(SignType::S_ROUND_CLOSE, /*stop@semi*/ true, 
      /*consume*/ true))
      return DeclResult::Error();
  }
  
  // [':' <type>]
  if (auto colon = consumeSign(SignType::S_COLON)) {
    if (auto rtrTy = parseType()) {
      TypeLoc tl = rtrTy.createTypeLoc();
      rtr->setReturnTypeLoc(tl);
    }
    else {
      if (rtrTy.wasSuccessful())
        reportErrorExpected(DiagID::parser_expected_type);

      if (!resyncToSign(SignType::S_CURLY_OPEN, true, false))
        return DeclResult::Error();
    }
  }
  // if no return type, the function returns void.
  // (We don't need to change anything since it's the default type
  //  set by the ctor)

  // <compound_statement>
  StmtResult compStmt = parseCompoundStatement();

  if (!compStmt) {
    if(compStmt.wasSuccessful()) // Display only if it was not found
      reportErrorExpected(DiagID::parser_expected_opening_curlybracket);
    return DeclResult::Error();
  }

  CompoundStmt* body = dyn_cast<CompoundStmt>(compStmt.get());
  assert(body && "Not a compound stmt");

  // Finished parsing. If the decl is invalid, return an error.
  if (invalid) return DeclResult::Error();

  // Restore the last decl parent.
  parentGuard.restore();

  // Create the full range for this FuncDecl
  SourceRange range(begLoc, body->getEnd());
  assert(range && begLoc && "Invalid loc info");

  // Finish building our FuncDecl.
  ParamList* paramList = ParamList::create(ctxt, params);
  rtr->setParams(paramList);
  rtr->setBody(body);
  // Record it
  actOnDecl(rtr);
  rtr->calculateValueType();
  assert(rtr->getValueType() && "FuncDecl type not calculated");
  return DeclResult(rtr);
}

Parser::DeclResult Parser::parseParamDecl() {
  // <param_decl> = <id> ':' ["mut"] <type>
  assert(isParsingFuncDecl() && "Can only call this when parsing a function!");

  // <id>
  auto id = consumeIdentifier();
  if (!id) return DeclResult::NotFound();

  // ':'
  if (!consumeSign(SignType::S_COLON)) {
    reportErrorExpected(DiagID::parser_expected_colon);
    return DeclResult::Error();
  }

  bool isMutable = (bool)consumeKeyword(KeywordType::KW_MUT);

  // <type>
  auto typeResult = parseType();
  if (!typeResult) {
    if (typeResult.wasSuccessful())
      reportErrorExpected(DiagID::parser_expected_type);
    return DeclResult::Error();
  }

  TypeLoc tl = typeResult.createTypeLoc();

  assert(id.getRange() && typeResult.getRange() && "Invalid loc info");

  auto* rtr = ParamDecl::create(ctxt, getDeclParent().get<FuncDecl*>(), 
    id.get(), id.getRange(), tl, isMutable);
  actOnDecl(rtr);
  return DeclResult(rtr);
}

Parser::DeclResult Parser::parseVarDecl() {
  // <var_decl> = ("let" | "var") <id> ':' <type> ['=' <expr>] ';'


  // ("let" | "var")
  VarDecl::Keyword kw;
  SourceLoc begLoc;
  if (auto letKw = consumeKeyword(KeywordType::KW_LET)) {
    kw = VarDecl::Keyword::Let;
    begLoc = letKw.getBegin();
  } 
  else if(auto varKw = consumeKeyword(KeywordType::KW_VAR)) {
    kw = VarDecl::Keyword::Var;
    begLoc = varKw.getBegin();
  }
  else
    return DeclResult::NotFound();
  
  // Helper lambda
  auto tryRecoveryToSemi = [&]() {
    if (resyncToSign(SignType::S_SEMICOLON, /*stop@semi*/false,
        /*consumeToken*/true)) {
      // If we recovered to a semicon, simply return not found.
      return DeclResult::NotFound();
    }
    // Else, return an error.
    return DeclResult::Error();
  };

  // <id>
  auto id = consumeIdentifier();
  if(!id) {
    reportErrorExpected(DiagID::parser_expected_iden);
    return tryRecoveryToSemi();
  }

  // ':'
  if (!consumeSign(SignType::S_COLON)) {
    reportErrorExpected(DiagID::parser_expected_colon);
    return DeclResult::Error();
  }

  // <type>
  TypeLoc type;
  if (auto qtRes = parseType())
    type = qtRes.createTypeLoc();
  else {
    if (qtRes.wasSuccessful())
      reportErrorExpected(DiagID::parser_expected_type);
    return tryRecoveryToSemi();
  }

  // ['=' <expr>]
  Expr* iExpr = nullptr;
  if (consumeSign(SignType::S_EQUAL)) {
    if (auto expr = parseExpr())
      iExpr = expr.get();
    else {
      if (expr.wasSuccessful())
        reportErrorExpected(DiagID::parser_expected_expr);
      // Recover to semicolon, return if recovery wasn't successful 
      if (!resyncToSign(SignType::S_SEMICOLON, 
				/*stop@semi*/ false, /*consumeToken*/ false))
        return DeclResult::Error();
    }
  }

  // ';'
  SourceLoc endLoc = consumeSign(SignType::S_SEMICOLON);
  if (!endLoc) {
    reportErrorExpected(DiagID::parser_expected_semi);
      
    if (!resyncToSign(SignType::S_SEMICOLON, 
			/*stopAtSemi*/ false, /*consumeToken*/ false))
      return DeclResult::Error();

    endLoc = consumeSign(SignType::S_SEMICOLON);
  }

  SourceRange range(begLoc, endLoc);
  assert(range && "Invalid loc info");
  assert(type.isComplete() && "Incomplete TypeLoc!");
  auto rtr = VarDecl::create(ctxt, getDeclParent(), id.get(), id.getRange(),
    type, kw, iExpr, range);

  actOnDecl(rtr);
  return DeclResult(rtr);
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