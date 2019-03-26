//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : ParseStmt.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Parser/Parser.hpp"
#include "Fox/AST/ASTNode.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"
#include "Fox/Common/LLVM.hpp"
#include "llvm/ADT/SmallVector.h"

using namespace fox;

Parser::Result<Stmt*> Parser::parseCompoundStatement() {
  // '{' {<stmt>} '}'

  // '{'
  auto lbrace = tryConsume(TokenKind::LBrace).getBeginLoc();

  if (!lbrace) return Result<Stmt*>::NotFound();

  // Once we're sure that we're in a CompoundStmt, activate the 
  // DelayedDeclRegistration.
  DelayedDeclRegistration ddr(this);

  // The nodes of this CompoundStmt
  SmallVector<ASTNode, 8> nodes;

  // The SourceLoc of the '}'
  SourceLoc rbrace;
  // {<stmt>}
  while (!isDone()) {
    // Try to consume the '}'
    // '}'
    if ((rbrace = tryConsume(TokenKind::RBrace).getBeginLoc()))
      break;

    // Try to parse a statement
    if(auto res = parseStmt())
      nodes.push_back(res.get());
    // Parsing failed
    else {
      // In both case, attempt recovery to nearest semicolon.
      // FIXME: Is this the right thing? Tests needed!
      if (resyncTo(TokenKind::Semi,/*stopAtSemi*/ false, /*consume*/ true))
        continue;
      else {
        // If we couldn't recover, try to recover to our '}'
        // to stop the parsing of this compound statement early.
        if (resyncTo(TokenKind::RBrace, 
                     /*stopAtSemi*/ false, /*consume*/ false)) {
          rbrace = tryConsume(TokenKind::RBrace).getBeginLoc();
          break;
        }
        // Couldn't recover, error.
        else return Result<Stmt*>::Error();
      }
    }
  }

  // '}'
  if (!rbrace.isValid()) {
    reportErrorExpected(DiagID::expected_closing_curly_bracket);
    // We can't recover since we probably reached EOF. return an error!
    return Result<Stmt*>::Error();
  }

  // Create & return the node
  SourceRange range(lbrace, rbrace);
  assert(range && "invalid loc info");
  auto* rtr = CompoundStmt::create(ctxt, nodes, range);

  // Complete the delayed decl registration and return the node.
  ddr.complete(ScopeInfo(rtr));
  return Result<Stmt*>(rtr);
}

Parser::Result<Stmt*> Parser::parseWhileLoop() {
  // <while_loop> = "while" <expr> <body>

  // "while"
  auto whKw = tryConsume(TokenKind::WhileKw);
  if (!whKw)
    return Result<Stmt*>::NotFound();

  // <expr>
  Expr* expr = nullptr;
  if (auto exprResult = parseExpr())
    expr = exprResult.get();
  else {
    reportErrorExpected(DiagID::expected_expr);
    return Result<Stmt*>::Error();
  }

  // <body>
  CompoundStmt* body = nullptr;
  if (auto body_res = parseCompoundStatement())
    body = body_res.castTo<CompoundStmt>();
  else {
    if (body_res.isNotFound())
      reportErrorExpected(DiagID::expected_opening_curly_bracket);
    return Result<Stmt*>::Error();
  }

  assert(expr && body->getEndLoc() && whKw.getBeginLoc());
  return Result<Stmt*>(
    WhileStmt::create(ctxt, whKw.getBeginLoc(), expr, body)
  );
}

Parser::Result<Stmt*> Parser::parseCondition() {
  // <condition> = "if" <expr> <compound_stmt> 
  //                ["else" (<compound_stmt> | <condition>)]
  Expr* expr = nullptr;
  CompoundStmt* then_body = nullptr;
  Stmt* else_node = nullptr;

  // "if"
  auto ifKw = tryConsume(TokenKind::IfKw);
  if (!ifKw) {
    // check for a else without if
    if (auto elseKw = tryConsume(TokenKind::ElseKw)) {
      diagEngine.report(DiagID::else_without_if, elseKw);
      return Result<Stmt*>::Error();
    }
    return Result<Stmt*>::NotFound();
  }

  // <expr>
  if (auto exprResult = parseExpr())
    expr = exprResult.get();
  else {
    reportErrorExpected(DiagID::expected_expr);
    return Result<Stmt*>::Error();
  }
    
  // <compound_stmt>
  if (auto body = parseCompoundStatement())
    then_body = body.castTo<CompoundStmt>();
  else {
    if (body.isNotFound())
      reportErrorExpected(DiagID::expected_opening_curly_bracket);
    return Result<Stmt*>::Error();
  }

  // "else"
  if (tryConsume(TokenKind::ElseKw)) {
    // <condition>
    if (auto cond = parseCondition())
      else_node = cond.castTo<ConditionStmt>();
    else if(cond.isError())
      return Result<Stmt*>::Error();

    // <compound_stmt>
    if (auto compound = parseCompoundStatement())
      else_node = compound.castTo<CompoundStmt>();
    else if(compound.isError())
      return Result<Stmt*>::Error();

    if (!else_node) {
      reportErrorExpected(DiagID::expected_opening_curly_bracket);
      return Result<Stmt*>::Error();
    }
  }

  assert(expr->getSourceRange() 
    && then_body->getSourceRange() 
    && ifKw.getBeginLoc() 
    && (else_node ? else_node->getSourceRange().isValid() : true) 
    && "incomplete locs");

  return Result<Stmt*>(
    ConditionStmt::create(ctxt, ifKw.getBeginLoc(), expr, then_body, else_node)
  );
}

Parser::Result<Stmt*> Parser::parseReturnStmt() {
  // <rtr_stmt> = "return" [<expr>] ';'
  // "return"
  auto rtrKw = tryConsume(TokenKind::ReturnKw);
  if (!rtrKw)
    return Result<Stmt*>::NotFound();
  
  Expr* expr = nullptr;
  SourceLoc begLoc = rtrKw.getBeginLoc();
  SourceLoc endLoc;

  // [<expr>]
  if (auto expr_res = parseExpr())
    expr = expr_res.get();
  else if(expr_res.isError()) {
    // expr failed? try to resync to a ';' if possible
    if (!resyncTo(TokenKind::Semi, /* stopAtSemi */ false, /*consume*/ true))
      return Result<Stmt*>::Error();
  }

  // ';'
  if (auto semi = tryConsume(TokenKind::Semi).getBeginLoc())
    endLoc = semi;
  else {
    reportErrorExpected(DiagID::expected_semi);
    // Recover to a ';', if recovery wasn't successful -> error
    if (!resyncTo(TokenKind::Semi, /* stopAtSemi */ false,  /*consume*/ true))
      return Result<Stmt*>::Error();
  }
    
  SourceRange range(begLoc, endLoc);
  assert(range && "Invalid loc info");
  return Result<Stmt*>(
    ReturnStmt::create(ctxt, expr, range)
  );
}

Parser::Result<ASTNode> Parser::parseStmt() {
  // <stmt>  = <var_decl> | <expr_stmt> | 
  //           <condition> | <while_loop> | <rtr_stmt> 

  // <var_decl
  if (auto vardecl = parseVarDecl())
    return Result<ASTNode>(ASTNode(vardecl.get()));
  else if (vardecl.isError())
    return Result<ASTNode>::Error();

  // <expr_stmt>
  if (auto exprstmt = parseExprStmt())
    return exprstmt;
  else if (exprstmt.isError())
    return Result<ASTNode>::Error();

  // <condition>
  if(auto cond = parseCondition())
    return Result<ASTNode>(ASTNode(cond.get()));
  else if (cond.isError())
    return Result<ASTNode>::Error();

  // <while_loop>
  if (auto wloop = parseWhileLoop())
    return Result<ASTNode>(ASTNode(wloop.get()));
  else if(wloop.isError())
    return Result<ASTNode>::Error();

  // <return_stmt>
  if (auto rtrstmt = parseReturnStmt())
    return Result<ASTNode>(ASTNode(rtrstmt.get()));
  else if(rtrstmt.isError())
    return Result<ASTNode>::Error();

  return Result<ASTNode>::NotFound();
}

Parser::Result<ASTNode> Parser::parseExprStmt() {
  // <expr_stmt>  = <expr> ';'   

  // <expr> 
  if (auto expr = parseExpr()) {
    // ';'
    if (!tryConsume(TokenKind::Semi)) {
      reportErrorExpected(DiagID::expected_semi);

      if (!resyncTo(TokenKind::Semi, /* stopAtSemi */ false, 
        /*consume*/ true))
        return Result<ASTNode>::Error();
      // if recovery was successful, just return like nothing has happened!
    }

    return Result<ASTNode>(ASTNode(expr.get()));
  }
  else if(expr.isError()) {
    // if the expression had an error, ignore it and try to recover to a semi.
    if (resyncTo(TokenKind::Semi, /*stopAtSemi*/ false, /*consume*/ true)) {
      return Result<ASTNode>::NotFound();
    }
    return Result<ASTNode>::Error();
  }

  return Result<ASTNode>::NotFound();
}