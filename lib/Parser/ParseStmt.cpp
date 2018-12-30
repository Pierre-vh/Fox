//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
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

Parser::StmtResult Parser::parseCompoundStatement() {
  // Range will be filled up later, see line 67
  auto leftCurlyLoc = consumeBracket(SignType::S_CURLY_OPEN);

  if (!leftCurlyLoc)
    return StmtResult::NotFound();
  SmallVector<ASTNode, 4> nodes;
  SourceLoc rightCurlyLoc;
  while (!isDone()) {
    if ((rightCurlyLoc = consumeBracket(SignType::S_CURLY_CLOSE)))
      break;

    // try to parse a statement
    if(auto res = parseStmt()) {
      // Push only if we don't have a standalone NullStmt
      // this is done to avoid stacking them up, and since they're a no-op in all cases
      // it's meaningless to ignore them.
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
      if (resyncToSign(SignType::S_SEMICOLON,/*stopAtSemi -> meaningless here*/ false, /*shouldConsumeToken*/ true))
        continue;
      else {
        // If we couldn't recover, try to recover to our '}' to stop parsing this compound statement
        if (resyncToSign(SignType::S_CURLY_CLOSE, /*stopAtSemi*/ false, /*consume*/ false)) {
          rightCurlyLoc = consumeBracket(SignType::S_CURLY_CLOSE);
          break;
        }
        else
          return StmtResult::Error();
      }
    }
  }

  if (!rightCurlyLoc.isValid()) {
    reportErrorExpected(DiagID::parser_expected_closing_curlybracket);
    // We can't recover since we probably reached EOF. return an error!
    return StmtResult::Error();
  }

  // Create & return the node
  SourceRange range(leftCurlyLoc, rightCurlyLoc);
  assert(range && "invalid loc info");
  auto* rtr = CompoundStmt::create(ctxt, nodes, range);
  return StmtResult(rtr);
}

Parser::StmtResult Parser::parseWhileLoop() {
  // <while_loop>  = "while"  <parens_expr> <body>
  // "while"
  auto whKw = consumeKeyword(KeywordType::KW_WHILE);
  if (!whKw)
    return StmtResult::NotFound();

  Expr* expr = nullptr;
  ASTNode body;
  SourceLoc begLoc = whKw.getBegin();
  SourceLoc endLoc;
  // <parens_expr>
  if (auto parens_expr_res = parseParensExpr(nullptr))
    expr = parens_expr_res.get();
  else {
    reportErrorExpected(DiagID::parser_expected_opening_roundbracket);
    return StmtResult::Error();
  }

  // <body>
  if (auto body_res = parseBody()) {
    body = body_res.get();
    endLoc = body.getEndLoc();
    assert(endLoc && "The body parsed successfully, but doesn't have a valid endLoc?");
  }
  else {
    if (body_res.wasSuccessful())
      reportErrorExpected(DiagID::parser_expected_stmt);

    return StmtResult::Error();
  }

  SourceRange range(begLoc, endLoc);
  assert(expr && body && range);
  return StmtResult(
    WhileStmt::create(ctxt, expr, body, range)
  );
}

Parser::StmtResult Parser::parseCondition() {
  // <condition>  = "if"  <parens_expr> <body> ["else" <body>]
  Expr* expr = nullptr;
  ASTNode then_node;
  ASTNode else_node;

  // "if"
  auto ifKw = consumeKeyword(KeywordType::KW_IF);
  if (!ifKw) {
    // check for a else without if
    if (auto elseKw = consumeKeyword(KeywordType::KW_ELSE)) {
      diags.report(DiagID::parser_else_without_if, elseKw);
      return StmtResult::Error();
    }
    return StmtResult::NotFound();
  }

  SourceLoc begLoc = ifKw.getBegin();

  // <parens_expr>
  if (auto parensexpr = parseParensExpr(nullptr))
    expr = parensexpr.get();
  else {
    reportErrorExpected(DiagID::parser_expected_opening_roundbracket);
    return StmtResult::Error();
  }
    
  SourceLoc endLoc;

  // <body>
  if (auto body = parseBody()) {
    then_node = body.get();
    endLoc = then_node.getEndLoc();
  }
  else {
    if (body.wasSuccessful())
      reportErrorExpected(DiagID::parser_expected_stmt);

    return StmtResult::Error();
  }

  // "else"
  if (consumeKeyword(KeywordType::KW_ELSE)) {
    // <body>
    if (auto body = parseBody()) {
      else_node = body.get();
      endLoc = else_node.getEndLoc();
    }
    else {
      if(body.wasSuccessful())
        reportErrorExpected(DiagID::parser_expected_stmt);
      return StmtResult::Error();
    }
  }

  SourceRange range(begLoc, endLoc);
  assert(expr && then_node && range && "Incomplete loc/nodes!");

  return StmtResult(
    ConditionStmt::create(ctxt, expr, then_node, else_node, range)
  );
}

Parser::StmtResult Parser::parseReturnStmt() {
  // <rtr_stmt> = "return" [<expr>] ';'
  // "return"
  auto rtrKw = consumeKeyword(KeywordType::KW_RETURN);
  if (!rtrKw)
    return StmtResult::NotFound();
  
  Expr* expr = nullptr;
  SourceLoc begLoc = rtrKw.getBegin();
  SourceLoc endLoc;

  // [<expr>]
  if (auto expr_res = parseExpr())
    expr = expr_res.get();
  else if(!expr_res.wasSuccessful()) {
    // expr failed? try to resync if possible. 
    if (!resyncToSign(SignType::S_SEMICOLON, /* stopAtSemi */ false, /*consumeToken*/ true))
      return StmtResult::Error();
  }

  // ';'
  if (auto semi = consumeSign(SignType::S_SEMICOLON))
    endLoc = semi;
  else {
    reportErrorExpected(DiagID::parser_expected_semi);
    // Recover to semi, if recovery wasn't successful, return an error.
    if (!resyncToSign(SignType::S_SEMICOLON, /* stopAtSemi */ false, /*consumeToken*/ true))
      return StmtResult::Error();
  }
    
  SourceRange range(begLoc, endLoc);
  assert(range && "Invalid loc info");
  return StmtResult(
    ReturnStmt::create(ctxt, expr, range)
  );
}

Parser::NodeResult Parser::parseStmt() {
  // <stmt>  = <var_decl> | <expr_stmt> | <condition> | <while_loop> | <rtr_stmt> 

  // <var_decl
  if (auto vardecl = parseVarDecl())
    return NodeResult(ASTNode(vardecl.get()));
  else if (!vardecl.wasSuccessful())
    return NodeResult::Error();

  // <expr_stmt>
  if (auto exprstmt = parseExprStmt())
    return exprstmt;
  else if (!exprstmt.wasSuccessful())
    return NodeResult::Error();

  // <condition>
  if(auto cond = parseCondition())
    return NodeResult(ASTNode(cond.get()));
  else if (!cond.wasSuccessful())
    return NodeResult::Error();

  // <while_loop>
  if (auto wloop = parseWhileLoop())
    return NodeResult(ASTNode(wloop.get()));
  else if(!wloop.wasSuccessful())
    return NodeResult::Error();

  // <return_stmt>
  if (auto rtrstmt = parseReturnStmt())
    return NodeResult(ASTNode(rtrstmt.get()));
  else if(!rtrstmt.wasSuccessful())
    return NodeResult::Error();

  return NodeResult::NotFound();
}

Parser::NodeResult Parser::parseBody() {
  // <body>  = <stmt> | <compound_statement>

  // <stmt>
  if (auto stmt = parseStmt())
    return stmt;
  else if (!stmt.wasSuccessful())
    return NodeResult::Error();

  // <compound_statement>
  if (auto compoundstmt = parseCompoundStatement())
    return NodeResult(ASTNode(compoundstmt.get()));
  else if (!compoundstmt.wasSuccessful())
    return NodeResult::Error();

  return NodeResult::NotFound();
}

Parser::NodeResult Parser::parseExprStmt() {
  // <expr_stmt>  = ';' | <expr> ';'   

  // ';'
  if (auto semi = consumeSign(SignType::S_SEMICOLON)) {
    Stmt* nullstmt = NullStmt::create(ctxt, semi);
    return NodeResult(ASTNode(nullstmt));
  }

  // <expr> 
  else if (auto expr = parseExpr()) {
    // ';'
    if (!consumeSign(SignType::S_SEMICOLON)) {
      if (expr.wasSuccessful())
        reportErrorExpected(DiagID::parser_expected_semi);

      if (!resyncToSign(SignType::S_SEMICOLON, /* stopAtSemi */ false, /*consumeToken*/ true))
        return NodeResult::Error();
      // if recovery was successful, just return like nothing has happened!
    }

    return NodeResult(ASTNode(expr.get()));
  }
  else if(!expr.wasSuccessful()) {
    // if the expression had an error, ignore it and try to recover to a semi.
    if (resyncToSign(SignType::S_SEMICOLON,
      /*stopAtSemi*/ false, /*consumeToken*/ false)) {
      Stmt* nullstmt = NullStmt::create(ctxt, consumeSign(SignType::S_SEMICOLON));
      return NodeResult(ASTNode(nullstmt));
    }
    return NodeResult::Error();
  }

  return NodeResult::NotFound();
}