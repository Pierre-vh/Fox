//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
// See LICENSE.txt for license info.            
// File : ParseExpr.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file implements expressions related methods (rules)  
//----------------------------------------------------------------------------//

#include "Fox/AST/ASTContext.hpp"
#include "Fox/Parser/Parser.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"

using namespace fox;

Parser::Result<Expr*> Parser::parseSuffix(Expr* base) {
  assert(base && "Base cannot be nullptr!");

  // <suffix> = '.' <id> | '[' <expr> ']' | <parens_expr_list>

  SourceLoc endLoc;

  // "." <id> 
  // '.'
  if (auto dotLoc = consumeSign(SignType::S_DOT)) {
    // <id>
    if (auto idRes = consumeIdentifier()) {
      // found, return
      Identifier id = idRes.getValue().first;
      SourceRange idRange = idRes.getValue().second;
      return Result<Expr*>(
        MemberOfExpr::create(ctxt, base , id, idRange, dotLoc)
      );
    }
    else  {
      reportErrorExpected(DiagID::parser_expected_iden);
      return Result<Expr*>::Error();
    }
  }
  // '[' <expr> ']
  // '['
  else if (consumeBracket(SignType::S_SQ_OPEN)) {
    // <expr>
    if (auto expr = parseExpr()) {
      // ']'
      SourceLoc rSqBrLoc = consumeBracket(SignType::S_SQ_CLOSE);
      if (!rSqBrLoc) {
        reportErrorExpected(DiagID::parser_expected_closing_squarebracket);

        if (resyncToSign(SignType::S_SQ_CLOSE, /* stopAtSemi */ true, 
          /*consumeToken*/ false))
          rSqBrLoc = consumeBracket(SignType::S_SQ_CLOSE);
        else
          return Result<Expr*>::Error();
      }


      return Result<Expr*>(
        ArraySubscriptExpr::create(ctxt, base, expr.get(), rSqBrLoc)
      );
    }
    else {
      if (expr.isNotFound())
        reportErrorExpected(DiagID::parser_expected_expr);

      // Resync. if Resync is successful, return the base as the result 
      // (don't alter it) to fake a success
      // , if it's not, return an Error.
      if (resyncToSign(SignType::S_SQ_CLOSE, /* stopAtSemi */ true, 
        /*consumeToken*/ true))
        return Result<Expr*>(base);
      else
        return Result<Expr*>::Error();
    }
  }
  // <parens_expr_list>
  else if (auto exprlist = parseParensExprList(&endLoc)) {
    assert(endLoc && "parseParensExprList didn't complete the endLoc?");
    return Result<Expr*>(
      CallExpr::create(ctxt, base, exprlist.move(), endLoc)
    );
  }
  else if (exprlist.isError())
    return Result<Expr*>::Error();
  return Result<Expr*>::NotFound();
}

Parser::Result<Expr*> Parser::parseDeclRef() {
  // <decl_call> = <id> 
  if (auto idRes = consumeIdentifier()) {
    Identifier id = idRes.getValue().first;
    SourceRange idRange = idRes.getValue().second;
    return Result<Expr*>(UnresolvedDeclRefExpr::create(ctxt, id,
      idRange));
  }
  return Result<Expr*>::NotFound();
}

Parser::Result<Expr*> Parser::parsePrimitiveLiteral() {
  // <primitive_literal>  = One literal of the following type : Integer,
  //                        Floating-point, Boolean, String, Char
  auto tok = getCurtok();
  if (!tok.isLiteral())
    return Result<Expr*>::NotFound();
  
  next();

  auto litinfo = tok.getLiteralInfo();
  Expr* expr = nullptr;

  SourceRange range = tok.getRange();
  assert(range && "Invalid loc info");

  if (litinfo.isBool())
    expr = BoolLiteralExpr::create(ctxt, litinfo.get<bool>(), range);
  else if (litinfo.isString()) {
    string_view copiedString = ctxt.allocateCopy(litinfo.get<std::string>());
    expr = StringLiteralExpr::create(ctxt, copiedString, range);
  }
  else if (litinfo.isChar())
    expr = CharLiteralExpr::create(ctxt, litinfo.get<FoxChar>(), range);
  else if (litinfo.isInt())
    expr = IntegerLiteralExpr::create(ctxt, litinfo.get<FoxInt>(), range);
  else if (litinfo.isFloat())
    expr = DoubleLiteralExpr::create(ctxt, litinfo.get<FoxDouble>(), range);
  else
    fox_unreachable("Unknown literal kind"); // Unknown literal

  return Result<Expr*>(expr);
}

Parser::Result<Expr*> Parser::parseArrayLiteral() {
  // <array_literal>  = '[' [<expr_list>] ']'
  auto begLoc = consumeBracket(SignType::S_SQ_OPEN);
  if (!begLoc)
    return Result<Expr*>::NotFound();
  
  // [<expr_list>]
  auto elist = parseExprList(); 

  // We don't check for errors because even if it failed, the Result object
  // will construct a empty ExprList for us!

  // ']'
  SourceLoc endLoc = consumeBracket(SignType::S_SQ_CLOSE);
  if (!endLoc) {
    if (elist.isNotFound())
      reportErrorExpected(DiagID::parser_expected_closing_squarebracket);

    if (resyncToSign(SignType::S_SQ_CLOSE, /* stopAtSemi */ true, 
      /*consumeToken*/ false))
      endLoc = consumeBracket(SignType::S_SQ_CLOSE);
    else
      return Result<Expr*>::Error();
  }

  SourceRange range(begLoc, endLoc);
  assert(range && "Invalid loc info");
  return Result<Expr*>(ArrayLiteralExpr::create(ctxt, elist.move(), range));
}

Parser::Result<Expr*> Parser::parseLiteral() {
  // <literal>  = <primitive_literal> | <array_literal>

  // <primitive_literal>
  if (auto prim = parsePrimitiveLiteral())
    return prim;
  else if (prim.isError())
    return Result<Expr*>::Error();

  // <array_literal>
  if (auto arr = parseArrayLiteral())
    return arr;
  else if (arr.isError())
    return Result<Expr*>::Error();

  return Result<Expr*>::NotFound();
}

Parser::Result<Expr*> Parser::parsePrimary() {
  // = <literal>
  if (auto lit = parseLiteral())
    return lit;
  else if(lit.isError())
    return Result<Expr*>::Error();

  // = <decl_call>
  if (auto declcall = parseDeclRef())
    return declcall;
  else if(declcall.isError())
    return Result<Expr*>::Error();

  // = '(' <expr> ')'
  if (auto parens_expr = parseParensExpr())
    return parens_expr;
  else if (parens_expr.isError())
    return Result<Expr*>::Error();

  return Result<Expr*>::NotFound();
}

Parser::Result<Expr*> Parser::parseSuffixExpr() {
  // <suffix_expr>  = <primary> { <suffix> }
  if (auto prim = parsePrimary()) {
    Expr* base = prim.get();
    Result<Expr*> suffix;
    while ((suffix = parseSuffix(base)))
      base = suffix.get();

    if (suffix.isNotFound())
      return Result<Expr*>(base);
    else
      return Result<Expr*>::Error();
  }
  else {
    if (prim.isError())
      return Result<Expr*>::Error();
    return Result<Expr*>::NotFound();
  }
}

Parser::Result<Expr*> Parser::parseExponentExpr() {
  // <exp_expr>  = <suffix_expr> [ <exponent_operator> <prefix_expr> ]

  // <suffix_expr>
  auto lhs = parseSuffixExpr();
  if (!lhs)
    return lhs; 

  // <exponent_operator> 
  if (auto expOp = parseExponentOp()) {
    // <prefix_expr>
    auto rhs = parsePrefixExpr();
    if (!rhs) {
      if(rhs.isNotFound())
        reportErrorExpected(DiagID::parser_expected_expr);
        
      return Result<Expr*>::Error();
    }

    return Result<Expr*>(BinaryExpr::create(ctxt, BinaryExpr::OpKind::Exp,
      lhs.get(), rhs.get(), expOp));
  }

  return lhs;
}

Parser::Result<Expr*> Parser::parsePrefixExpr() {
  // <prefix_expr>  = <unary_operator> <prefix_expr> | <exp_expr>

	// <unary_operator>
  SourceRange opRange;
  if (auto uop = parseUnaryOp(opRange)) {
		// <prefix_expr>
    if (auto prefixexpr = parsePrefixExpr()) {
      return Result<Expr*>(
        UnaryExpr::create(ctxt, uop.get(), prefixexpr.get(),opRange));
    }
    else {
      if(prefixexpr.isNotFound())
        reportErrorExpected(DiagID::parser_expected_expr);

      return Result<Expr*>::Error();
    }
  }

  // <exp_expr>
  if (auto expExpr = parseExponentExpr())
    return expExpr;
  else if (expExpr.isError())
    return Result<Expr*>::Error();

  return Result<Expr*>::NotFound();
}

Parser::Result<Expr*> Parser::parseCastExpr() {
  // <cast_expr>  = <prefix_expr> ["as" <type>]
  // <cast_expr>
  auto prefixexpr = parsePrefixExpr();
  if (!prefixexpr) {
    if (prefixexpr.isError())
      return Result<Expr*>::Error();
    return Result<Expr*>::NotFound();
  }

  // ["as" <type>]
  if (consumeKeyword(KeywordType::KW_AS)) {
    // <type>
    if (auto tyRes = parseType()) {
      TypeLoc tl = tyRes.get();
      SourceLoc begLoc = prefixexpr.get()->getBegin();
      SourceLoc endLoc = tl.getEnd();

      SourceRange range(begLoc, endLoc);
      assert(range && "Invalid loc info");

      return Result<Expr*>(CastExpr::create(ctxt, tl, prefixexpr.get()));
    }
    else {
      reportErrorExpected(DiagID::parser_expected_type);
      return Result<Expr*>::Error();
    }
  }

  return prefixexpr;
}

Parser::Result<Expr*> Parser::parseBinaryExpr(std::uint8_t precedence) {
  // <binary_expr>  = <cast_expr> { <binary_operator> <cast_expr> }  

  // <cast_expr> OR a binaryExpr of inferior priority.
  Result<Expr*> lhsResult;
  if (precedence > 0)
    lhsResult = parseBinaryExpr(precedence - 1);
  else
    lhsResult = parseCastExpr();

  if (!lhsResult) {
    if (lhsResult.isError())
      return Result<Expr*>::Error();
    return Result<Expr*>::NotFound();
  }

  Expr* lhs = lhsResult.get();
  BinaryExpr* rtr = nullptr;
  
  // { <binary_operator> <cast_expr> }  
  while (true) {
    // <binary_operator>
    SourceRange opRange;
    auto binop_res = parseBinaryOp(precedence, opRange);
    if (!binop_res) // No operator found : break.
      break;

    // <cast_expr> OR a binaryExpr of inferior priority.
    Result<Expr*> rhsResult;
    if (precedence > 0)
      rhsResult = parseBinaryExpr(precedence - 1);
    else
      rhsResult = parseCastExpr();


    // Handle results appropriately
		// Check for validity : we need a rhs. if we don't have one, 
    // we have an error ! 
    if (!rhsResult) {
      if(rhsResult.isNotFound())
        reportErrorExpected(DiagID::parser_expected_expr);
      return Result<Expr*>::Error();
    }

    Expr* rhs = rhsResult.get();

    rtr = BinaryExpr::create(ctxt, binop_res.get(), 
      (rtr ? rtr : lhs), rhs, opRange);
  }

  if (!rtr) {
    assert(lhs && "no rtr node + no lhs node?");
    return Result<Expr*>(lhs);
  }
  return Result<Expr*>(rtr);
}

Parser::Result<Expr*> Parser::parseExpr() {
  //  <expr> = <binary_expr> [<assign_operator> <expr>] 
  auto lhs = parseBinaryExpr();
  if (!lhs)
    return lhs;

  SourceRange opRange;
  if (auto op = parseAssignOp(opRange)) {
    auto rhs = parseExpr();
    if (!rhs) {
      if(rhs.isNotFound())
        reportErrorExpected(DiagID::parser_expected_expr);
      return Result<Expr*>::Error();
    }

    return Result<Expr*>(BinaryExpr::create(ctxt, op.get(), 
      lhs.get(), rhs.get(), opRange));
  }
  return Result<Expr*>(lhs);
}

Parser::Result<Expr*> Parser::parseParensExpr() {
  // <parens_expr> = '(' <expr> ')'

  // '('
  auto leftParens = consumeBracket(SignType::S_ROUND_OPEN);
  if (!leftParens)
    return Result<Expr*>::NotFound();

  Expr* rtr = nullptr;
    
  // <expr>
  if (auto expr = parseExpr())
    rtr = expr.get();
  else  {
    // no expr, handle error & attempt to recover if it's allowed. 
    // If recovery is successful, return "not found"
    if(expr.isNotFound())
      reportErrorExpected(DiagID::parser_expected_expr);

    if (resyncToSign(SignType::S_ROUND_CLOSE, /* stopAtSemi */ true,
      /*consumeToken*/ true))
      return Result<Expr*>::NotFound();
    else
      return Result<Expr*>::Error();
  }

  assert(rtr && "The return value shouldn't be null!");

  // ')'
  auto rightParens = consumeBracket(SignType::S_ROUND_CLOSE);
  if (!rightParens) {
    // no ), handle error & attempt to recover 
    reportErrorExpected(DiagID::parser_expected_closing_roundbracket);

    if (!resyncToSign(SignType::S_ROUND_CLOSE, /* stopAtSemi */ true, 
      /*consumeToken*/ false))
      return Result<Expr*>::Error();
      
    // If we recovered successfuly, place the Sloc into rightParens
    rightParens = consumeBracket(SignType::S_ROUND_CLOSE);
  }

  return Result<Expr*>(rtr);
}

Parser::Result<ExprVector> Parser::parseExprList() {
  // <expr_list> = <expr> {',' <expr> }
  auto firstexpr = parseExpr();
  if (!firstexpr)
    return Result<ExprVector>::NotFound();

  ExprVector exprs;
  exprs.push_back(firstexpr.get());
  while (auto comma = consumeSign(SignType::S_COMMA)) {
    if (auto expr = parseExpr())
      exprs.push_back(expr.get());
    else {
      if (expr.isNotFound()) {
        // if the expression was just not found, revert the comma consuming and
        // let the caller deal with the extra comma after the expression list.
        revertConsume();
        break;
      }

      return Result<ExprVector>::Error();
    }
  }

  return Result<ExprVector>(exprs);
}

Parser::Result<ExprVector> Parser::parseParensExprList(SourceLoc *RParenLoc) {
  // <parens_expr_list>  = '(' [ <expr_list> ] ')'
  // '('
  auto leftParens = consumeBracket(SignType::S_ROUND_OPEN);
  if (!leftParens)
    return Result<ExprVector>::NotFound();

  ExprVector exprs;

  //  [ <expr_list> ]
  if (auto exprlist = parseExprList())
    exprs = exprlist.get();
  else if (exprlist.isError()) {
    // error? Try to recover from it, if success, just discard the expr list,
    // if no success return error.
    if (resyncToSign(SignType::S_ROUND_CLOSE, /* stopAtSemi */ true,
      /*consumeToken*/ false)) {
      SourceLoc loc = consumeBracket(SignType::S_ROUND_CLOSE);

      if (RParenLoc)
        *RParenLoc = loc;

        // if recovery is successful, return an empty expression list.
      return Result<ExprVector>(ExprVector());
    }
    return Result<ExprVector>::Error();
  }

  SourceLoc rightParens = consumeBracket(SignType::S_ROUND_CLOSE);
  // ')'
  if (!rightParens) {
    reportErrorExpected(DiagID::parser_expected_closing_roundbracket);

    if (resyncToSign(SignType::S_ROUND_CLOSE, /* stopAtSemi */ true, 
      /*consumeToken*/ false))
      rightParens = consumeBracket(SignType::S_ROUND_CLOSE);
    else 
      return Result<ExprVector>::Error();
  }

  if (RParenLoc)
    *RParenLoc = rightParens;

  return Result<ExprVector>(exprs);
}

SourceRange Parser::parseExponentOp() {
  if (auto t1 = consumeSign(SignType::S_ASTERISK)) {
    if (auto t2 = consumeSign(SignType::S_ASTERISK))
      return SourceRange(t1,t2);
    revertConsume();
  }
  return SourceRange();
}

Parser::Result<BinaryExpr::OpKind> 
Parser::parseAssignOp(SourceRange& range) {
  using BinOp = BinaryExpr::OpKind;

  auto success = [&](BinOp op, SourceRange opRange) {
    range = opRange;
    return Result<BinOp>(op);
  };

  if (auto equal = consumeSign(SignType::S_EQUAL)) {
    // Try to match a S_EQUAL. If failed, that means that the next token isn't a =
    // If it succeeds, we found a '==' (=> this is the comparison operator) and
    // we must undo 
    if (!consumeSign(SignType::S_EQUAL))
      return success(BinOp::Assign, SourceRange(equal));
    undo();
  }
  return Result<BinOp>::NotFound();
}

Parser::Result<UnaryExpr::OpKind> 
Parser::parseUnaryOp(SourceRange& range) {
  using UOp = UnaryExpr::OpKind;

  auto success = [&](UOp op, SourceRange opRange) {
    range = opRange;
    return Result<UOp>(op);
  };

  if (auto excl = consumeSign(SignType::S_EXCL_MARK))
    return success(UOp::LNot, SourceRange(excl));
  else if (auto minus = consumeSign(SignType::S_MINUS))
    return success(UOp::Minus, SourceRange(minus));
  else if (auto plus = consumeSign(SignType::S_PLUS))
    return success(UOp::Plus, SourceRange(plus));
  return Result<UOp>::NotFound();
}

// TODO: This should be handled by the lexer, not the parser.
Parser::Result<BinaryExpr::OpKind> 
Parser::parseBinaryOp(std::uint8_t priority, SourceRange& range) {
  using BinOp = BinaryExpr::OpKind;

  // Check current Token validity, also check if it's a sign because if 
  // it isn't we can return directly!
  if (!getCurtok().isValid() || !getCurtok().isSign())
    return Result<BinOp>::NotFound();

  auto success = [&](BinOp op, SourceRange opRange) {
    range = opRange;
    return Result<BinOp>(op);
  };

  switch (priority) {
    case 0: // * / %
      if (auto asterisk = consumeSign(SignType::S_ASTERISK)) {
        // Disambiguation between '**' and '*'
        if (!consumeSign(SignType::S_ASTERISK))
          return success(BinOp::Mul, SourceRange(asterisk));
        // undo if not found
        undo();
      }
      else if (auto slash = consumeSign(SignType::S_SLASH))
        return success(BinOp::Div, SourceRange(slash));
      else if (auto percent = consumeSign(SignType::S_PERCENT))
        return success(BinOp::Mod, SourceRange(percent));
      break;
    case 1: // + -
      if (auto plus = consumeSign(SignType::S_PLUS))
        return success(BinOp::Add, SourceRange(plus));
      else if (auto minus = consumeSign(SignType::S_MINUS))
        return success(BinOp::Sub, SourceRange(minus));
      break;
    case 2: // > >= < <=
      if (auto lessthan = consumeSign(SignType::S_LESS_THAN)) {
        if (auto equal = consumeSign(SignType::S_EQUAL))
          return success(BinOp::LE, SourceRange(lessthan,equal));
        return success(BinOp::LT, SourceRange(lessthan));
      }
      else if (auto grthan = consumeSign(SignType::S_GREATER_THAN)) {
        if (auto equal = consumeSign(SignType::S_EQUAL))
          return success(BinOp::GE, SourceRange(grthan,equal));
        return success(BinOp::GT, SourceRange(grthan));
      }
      break;
    case 3:  // == !=
      // try to match '=' twice.
      if (auto equal1 = consumeSign(SignType::S_EQUAL)) {
        if (auto equal2 = consumeSign(SignType::S_EQUAL))
          return success(BinOp::Eq, SourceRange(equal1,equal2));
        // undo if not found
        undo();
      }
      else if (auto excl = consumeSign(SignType::S_EXCL_MARK)) {
        if (auto equal =consumeSign(SignType::S_EQUAL))
          return success(BinOp::NEq, SourceRange(excl,equal));
        // undo if not found
        undo();
      }
      break;
    case 4: // &&
      if (auto amp1 = consumeSign(SignType::S_AMPERSAND)) {
        if (auto amp2 = consumeSign(SignType::S_AMPERSAND))
          return success(BinOp::LAnd, SourceRange(amp1,amp2));
      }
      break;
    case 5: // ||
      if (auto vbar1 = consumeSign(SignType::S_VBAR)) {
        if (auto vbar2 = consumeSign(SignType::S_VBAR))
          return success(BinOp::LOr, SourceRange(vbar1,vbar2));
        // undo if not found
        undo();
      }
      break;
    default:
      fox_unreachable("Unknown priority");
      break;
  }
  return Result<BinOp>::NotFound();
}