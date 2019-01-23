//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
// See LICENSE.txt for license info.            
// File : ParseStmt.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  This file implements statements rules. parseStmt, parseVarDeclstmt,etc.                  
//----------------------------------------------------------------------------//

#include "Fox/Parser/Parser.hpp"
#include "Fox/AST/ASTNode.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"
#include "Fox/Common/LLVM.hpp"
#include "llvm/ADT/SmallVector.h"

using namespace fox;

Parser::Result<Stmt*> Parser::parseCompoundStatement() {
  // Range will be filled up later, see line 67
  auto leftCurlyLoc = consumeBracket(SignType::S_CURLY_OPEN);

  if (!leftCurlyLoc)
    return Result<Stmt*>::NotFound();
  SmallVector<ASTNode, 4> nodes;
  SourceLoc rightCurlyLoc;
  while (!isDone()) {
    if ((rightCurlyLoc = consumeBracket(SignType::S_CURLY_CLOSE)))
      break;

    // try to parse a statement
    if(auto res = parseStmt()) {
      // Push only if we don't have a standalone NullStmt
      // this is done to avoid stacking them up, and since they're 
      // a no-op in all cases so it's meaningless to ignore them.
      ASTNode node = res.get();
      if (!dyn_cast_or_null<NullStmt>(node.dyn_cast<Stmt*>()))
        nodes.push_back(node);
    }
    // failure
    else {
      /*
        // if not found, report an error
        if (stmt.wasSuccessful())
          errorExpected("Expected a Statement");
      */
      // In both case, attempt recovery to nearest semicolon.
      if (resyncToSign(SignType::S_SEMICOLON,/*stopAtSemi*/ false,
        /*shouldConsumeToken*/ true))
        continue;
      else {
        // If we couldn't recover, try to recover to our '}'
        // to stop parsing this compound statement
        if (resyncToSign(SignType::S_CURLY_CLOSE, /*stopAtSemi*/ false, 
          /*consume*/ false)) {
          rightCurlyLoc = consumeBracket(SignType::S_CURLY_CLOSE);
          break;
        }
        else
          return Result<Stmt*>::Error();
      }
    }
  }

  if (!rightCurlyLoc.isValid()) {
    reportErrorExpected(DiagID::parser_expected_closing_curlybracket);
    // We can't recover since we probably reached EOF. return an error!
    return Result<Stmt*>::Error();
  }

  // Create & return the node
  SourceRange range(leftCurlyLoc, rightCurlyLoc);
  assert(range && "invalid loc info");
  auto* rtr = CompoundStmt::create(ctxt, nodes, range);
  return Result<Stmt*>(rtr);
}

Parser::Result<Stmt*> Parser::parseWhileLoop() {
  // <while_loop> = "while" <expr> <body>

  // "while"
  auto whKw = consumeKeyword(KeywordType::KW_WHILE);
  if (!whKw)
    return Result<Stmt*>::NotFound();

  // <expr>
  Expr* expr = nullptr;
  if (auto exprResult = parseExpr())
    expr = exprResult.get();
  else {
    reportErrorExpected(DiagID::parser_expected_expr);
    return Result<Stmt*>::Error();
  }

  // <body>
  CompoundStmt* body = nullptr;
  if (auto body_res = parseCompoundStatement())
    body = body_res.castTo<CompoundStmt>();
  else {
    if (body_res.wasSuccessful())
      reportErrorExpected(DiagID::parser_expected_opening_curlybracket);
    return Result<Stmt*>::Error();
  }

  assert(expr && body->getEnd() && whKw.getBegin());
  return Result<Stmt*>(
    WhileStmt::create(ctxt, whKw.getBegin(), expr, body)
  );
}

Parser::Result<Stmt*> Parser::parseCondition() {
  // <condition> = "if" <expr> <compound_stmt> ["else" <compound_stmt>]
  Expr* expr = nullptr;
  CompoundStmt* then_body = nullptr;
  CompoundStmt* else_body = nullptr;

  // "if"
  auto ifKw = consumeKeyword(KeywordType::KW_IF);
  if (!ifKw) {
    // check for a else without if
    if (auto elseKw = consumeKeyword(KeywordType::KW_ELSE)) {
      diags.report(DiagID::parser_else_without_if, elseKw);
      return Result<Stmt*>::Error();
    }
    return Result<Stmt*>::NotFound();
  }

  // <expr>
  if (auto exprResult = parseExpr())
    expr = exprResult.get();
  else {
    reportErrorExpected(DiagID::parser_expected_expr);
    return Result<Stmt*>::Error();
  }
    
  // <compound_stmt>
  if (auto body = parseCompoundStatement())
    then_body = body.castTo<CompoundStmt>();
  else {
    if (body.wasSuccessful())
      reportErrorExpected(DiagID::parser_expected_opening_curlybracket);
    return Result<Stmt*>::Error();
  }

  // "else"
  if (consumeKeyword(KeywordType::KW_ELSE)) {
    // <compound_stmt>
    if (auto body = parseCompoundStatement())
      else_body = body.castTo<CompoundStmt>();
    else {
      if(body.wasSuccessful())
        reportErrorExpected(DiagID::parser_expected_opening_curlybracket);
      return Result<Stmt*>::Error();
    }
  }

  assert(expr->getRange() && then_body->getRange() && ifKw.getBegin() 
    && (else_body ? else_body->getRange().isValid() : true) 
    && "incomplete locs");

  return Result<Stmt*>(
    ConditionStmt::create(ctxt, ifKw.getBegin(), expr, then_body, else_body)
  );
}

Parser::Result<Stmt*> Parser::parseReturnStmt() {
  // <rtr_stmt> = "return" [<expr>] ';'
  // "return"
  auto rtrKw = consumeKeyword(KeywordType::KW_RETURN);
  if (!rtrKw)
    return Result<Stmt*>::NotFound();
  
  Expr* expr = nullptr;
  SourceLoc begLoc = rtrKw.getBegin();
  SourceLoc endLoc;

  // [<expr>]
  if (auto expr_res = parseExpr())
    expr = expr_res.get();
  else if(!expr_res.wasSuccessful()) {
    // expr failed? try to resync if possible. 
    if (!resyncToSign(SignType::S_SEMICOLON, /* stopAtSemi */ false, 
      /*consumeToken*/ true))
      return Result<Stmt*>::Error();
  }

  // ';'
  if (auto semi = consumeSign(SignType::S_SEMICOLON))
    endLoc = semi;
  else {
    reportErrorExpected(DiagID::parser_expected_semi);
    // Recover to semi, if recovery wasn't successful, return an error.
    if (!resyncToSign(SignType::S_SEMICOLON, /* stopAtSemi */ false, 
      /*consumeToken*/ true))
      return Result<Stmt*>::Error();
  }
    
  SourceRange range(begLoc, endLoc);
  assert(range && "Invalid loc info");
  return Result<Stmt*>(
    ReturnStmt::create(ctxt, expr, range)
  );
}

Parser::Result<ASTNode> Parser::parseStmt() {
  // <stmt>  = <var_decl> | <expr_stmt> | <condition> | <while_loop> | <rtr_stmt> 

  // <var_decl
  if (auto vardecl = parseVarDecl())
    return Result<ASTNode>(ASTNode(vardecl.get()));
  else if (!vardecl.wasSuccessful())
    return Result<ASTNode>::Error();

  // <expr_stmt>
  if (auto exprstmt = parseExprStmt())
    return exprstmt;
  else if (!exprstmt.wasSuccessful())
    return Result<ASTNode>::Error();

  // <condition>
  if(auto cond = parseCondition())
    return Result<ASTNode>(ASTNode(cond.get()));
  else if (!cond.wasSuccessful())
    return Result<ASTNode>::Error();

  // <while_loop>
  if (auto wloop = parseWhileLoop())
    return Result<ASTNode>(ASTNode(wloop.get()));
  else if(!wloop.wasSuccessful())
    return Result<ASTNode>::Error();

  // <return_stmt>
  if (auto rtrstmt = parseReturnStmt())
    return Result<ASTNode>(ASTNode(rtrstmt.get()));
  else if(!rtrstmt.wasSuccessful())
    return Result<ASTNode>::Error();

  return Result<ASTNode>::NotFound();
}

Parser::Result<ASTNode> Parser::parseExprStmt() {
  // <expr_stmt>  = ';' | <expr> ';'   

  // ';'
  if (auto semi = consumeSign(SignType::S_SEMICOLON)) {
    Stmt* nullstmt = NullStmt::create(ctxt, semi);
    return Result<ASTNode>(ASTNode(nullstmt));
  }

  // <expr> 
  else if (auto expr = parseExpr()) {
    // ';'
    if (!consumeSign(SignType::S_SEMICOLON)) {
      if (expr.wasSuccessful())
        reportErrorExpected(DiagID::parser_expected_semi);

      if (!resyncToSign(SignType::S_SEMICOLON, /* stopAtSemi */ false, 
        /*consumeToken*/ true))
        return Result<ASTNode>::Error();
      // if recovery was successful, just return like nothing has happened!
    }

    return Result<ASTNode>(ASTNode(expr.get()));
  }
  else if(!expr.wasSuccessful()) {
    // if the expression had an error, ignore it and try to recover to a semi.
    if (resyncToSign(SignType::S_SEMICOLON,
      /*stopAtSemi*/ false, /*consumeToken*/ false)) {
      Stmt* nullstmt = NullStmt::create(ctxt, 
        consumeSign(SignType::S_SEMICOLON));
      return Result<ASTNode>(ASTNode(nullstmt));
    }
    return Result<ASTNode>::Error();
  }

  return Result<ASTNode>::NotFound();
}