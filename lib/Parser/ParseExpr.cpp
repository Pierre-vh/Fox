//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : ParseExpr.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file implements expressions related methods (rules)  
//----------------------------------------------------------------------------//

#include "Fox/Parser/Parser.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"

using namespace fox;

Parser::ExprResult Parser::parseSuffix(Expr* base) {
  assert(base && "Base cannot be nullptr!");

  // <suffix> = '.' <id> | '[' <expr> ']' | <parens_expr_list>
  SourceLoc begLoc = base->getRange().getBegin();
  SourceLoc endLoc;
  // "." <id> 
  // '.'
  if (auto dotLoc = consumeSign(SignType::S_DOT)) {
    // <id>
    if (auto id = consumeIdentifier()) {
      // found, return
      endLoc = id.getRange().getEnd();

      SourceRange range(begLoc, endLoc);
      assert(range && dotLoc && "Invalid loc info");

      return ExprResult(
        MemberOfExpr::create(ctxt_, base ,id.get(), range, dotLoc)
      );
    }
    else  {
      reportErrorExpected(DiagID::parser_expected_iden);
      return ExprResult::Error();
    }
  }
  // '[' <expr> ']
  // '['
  else if (consumeBracket(SignType::S_SQ_OPEN)) {
    // <expr>
    if (auto expr = parseExpr()) {
      // ']'
      endLoc = consumeBracket(SignType::S_SQ_CLOSE);
      if (!endLoc) {
        reportErrorExpected(DiagID::parser_expected_closing_squarebracket);

        if (resyncToSign(SignType::S_SQ_CLOSE, /* stopAtSemi */ true, /*consumeToken*/ false))
          endLoc = consumeBracket(SignType::S_SQ_CLOSE);
        else
          return ExprResult::Error();
      }

      SourceRange range(begLoc, endLoc);
      assert(range && "Invalid loc info");

      return ExprResult(
        ArraySubscriptExpr::create(ctxt_, base, expr.get(), range)
      );
    }
    else {
      if (expr.wasSuccessful())
        reportErrorExpected(DiagID::parser_expected_expr);

      // Resync. if Resync is successful, return the base as the result (don't alter it) to fake a success
      // , if it's not, return an Error.
      if (resyncToSign(SignType::S_SQ_CLOSE, /* stopAtSemi */ true, /*consumeToken*/ true))
        return ExprResult(base);
      else
        return ExprResult::Error();
    }
  }
  // <parens_expr_list>
  else if (auto exprlist = parseParensExprList(nullptr,&endLoc)) {
    assert(endLoc && "parseParensExprList didn't complete the endLoc?");
    SourceRange range(begLoc, endLoc);
    assert(range && "Invalid loc info");
    return ExprResult(
      FunctionCallExpr::create(ctxt_, base, exprlist.getRef(), range)
    );
  }
  else if (!exprlist.wasSuccessful())
    return ExprResult::Error();
  return ExprResult::NotFound();
}

Parser::ExprResult Parser::parseDeclRef() {
  // <decl_call> = <id> 
  if (auto id = consumeIdentifier())
    return ExprResult(UnresolvedDeclRefExpr::create(ctxt_, id.get(),
      id.getRange()));
  return ExprResult::NotFound();
}

Parser::ExprResult Parser::parsePrimitiveLiteral() {
  // <primitive_literal>  = One literal of the following type : Integer,
  //                        Floating-point, Boolean, String, Char
  auto tok = getCurtok();
  if (!tok.isLiteral())
    return ExprResult::NotFound();
  
  next();

  auto litinfo = tok.getLiteralInfo();
  Expr* expr = nullptr;

  SourceRange range = tok.getRange();
  assert(range && "Invalid loc info");

  if (litinfo.isBool())
    expr = BoolLiteralExpr::create(ctxt_, litinfo.get<FoxBool>(), range);
  else if (litinfo.isString())
    expr = StringLiteralExpr::create(ctxt_, litinfo.get<FoxString>(), range);
  else if (litinfo.isChar())
    expr = CharLiteralExpr::create(ctxt_, litinfo.get<FoxChar>(), range);
  else if (litinfo.isInt())
    expr = IntegerLiteralExpr::create(ctxt_, litinfo.get<FoxInt>(), range);
  else if (litinfo.isFloat())
    expr = FloatLiteralExpr::create(ctxt_, litinfo.get<FoxFloat>(), range);
  else
    fox_unreachable("Unknown literal kind"); // Unknown literal

  return ExprResult(expr);
}

Parser::ExprResult Parser::parseArrayLiteral() {
  // <array_literal>  = '[' [<expr_list>] ']'
  auto begLoc = consumeBracket(SignType::S_SQ_OPEN);
  if (!begLoc)
    return ExprResult::NotFound();
  
  // [<expr_list>]
  auto elist = parseExprList(); 
  // We don't check for errors because even if it failed, the Result object
  // will construct a empty one for us!

  // ']'
  SourceLoc endLoc = consumeBracket(SignType::S_SQ_CLOSE);
  if (!endLoc) {
    if (elist.wasSuccessful())
      reportErrorExpected(DiagID::parser_expected_closing_squarebracket);

    if (resyncToSign(SignType::S_SQ_CLOSE, /* stopAtSemi */ true, /*consumeToken*/ false))
      endLoc = consumeBracket(SignType::S_SQ_CLOSE);
    else
      return ExprResult::Error();
  }

  SourceRange range(begLoc, endLoc);
  assert(range && "Invalid loc info");

  return ExprResult(ArrayLiteralExpr::create(ctxt_, elist.getRef(), range));
}

Parser::ExprResult Parser::parseLiteral() {
  // <literal>  = <primitive_literal> | <array_literal>

  // <primitive_literal>
  if (auto prim = parsePrimitiveLiteral())
    return prim;
  else if (!prim.wasSuccessful())
    return ExprResult::Error();

  // <array_literal>
  if (auto arr = parseArrayLiteral())
    return arr;
  else if (!arr.wasSuccessful())
    return ExprResult::Error();

  return ExprResult::NotFound();
}

Parser::ExprResult Parser::parsePrimary() {
  // = <literal>
  if (auto lit = parseLiteral())
    return lit;
  else if(!lit.wasSuccessful())
    return ExprResult::Error();

  // = <decl_call>
  if (auto declcall = parseDeclRef())
    return declcall;
  else if(!declcall.wasSuccessful())
    return ExprResult::Error();

  // = '(' <expr> ')'
  if (auto parens_expr = parseParensExpr())
    return parens_expr;
  else if (!parens_expr.wasSuccessful())
    return ExprResult::Error();

  return ExprResult::NotFound();
}

Parser::ExprResult Parser::parseSuffixExpr() {
  // <suffix_expr>  = <primary> { <suffix> }
  if (auto prim = parsePrimary()) {
    Expr* base = prim.get();
    ExprResult suffix;
    while (suffix = parseSuffix(base))
      base = suffix.get();

    if (suffix.wasSuccessful())
      return ExprResult(base);
    else
      return ExprResult::Error();
  }
  else {
    if (!prim.wasSuccessful())
      return ExprResult::Error();
    return ExprResult::NotFound();
  }
}

Parser::ExprResult Parser::parseExponentExpr() {
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
      if(rhs.wasSuccessful())
        reportErrorExpected(DiagID::parser_expected_expr);
        
      return ExprResult::Error();
    }

    SourceRange range = lhs.get()->getRange();
    assert(range && "Invalid loc info");

    return ExprResult(BinaryExpr::create(ctxt_, BinaryExpr::OpKind::Exp,
      lhs.get(), rhs.get(), range, expOp));
  }

  return lhs;
}

Parser::ExprResult Parser::parsePrefixExpr() {
  // <prefix_expr>  = <unary_operator> <prefix_expr> | <exp_expr>

	// <unary_operator>
  if (auto uop = parseUnaryOp()) {
		// <prefix_expr>
    if (auto prefixexpr = parsePrefixExpr()) {
      SourceLoc endLoc = prefixexpr.get()->getRange().getEnd();

      SourceRange range(uop.getRange().getBegin(), endLoc);
      assert(range && "Invalid loc info");

      return ExprResult(UnaryExpr::create(ctxt_, uop.get(), prefixexpr.get(),
          range, uop.getRange()));
    }
    else {
      if(prefixexpr.wasSuccessful())
        reportErrorExpected(DiagID::parser_expected_expr);

      return ExprResult::Error();
    }
  }

  // <exp_expr>
  if (auto expExpr = parseExponentExpr())
    return expExpr;
  else if (!expExpr.wasSuccessful())
    return ExprResult::Error();

  return ExprResult::NotFound();
}

Parser::ExprResult Parser::parseCastExpr() {
  // <cast_expr>  = <prefix_expr> ["as" <type>]
  // <cast_expr>
  auto prefixexpr = parsePrefixExpr();
  if (!prefixexpr) {
    if (!prefixexpr.wasSuccessful())
      return ExprResult::Error();
    return ExprResult::NotFound();
  }

  // ["as" <type>]
  if (consumeKeyword(KeywordType::KW_AS)) {
    // <type>
    if (auto tyRes = parseType()) {
      TypeLoc tl = tyRes.createTypeLoc();
      SourceLoc begLoc = prefixexpr.get()->getRange().getBegin();
      SourceLoc endLoc = tl.getRange().getEnd();

      SourceRange range(begLoc, endLoc);
      assert(range && "Invalid loc info");

      return ExprResult(CastExpr::create(ctxt_, tl, prefixexpr.get(),
          range));
    }
    else {
      reportErrorExpected(DiagID::parser_expected_type);
      return ExprResult::Error();
    }
  }

  return prefixexpr;
}

Parser::ExprResult Parser::parseBinaryExpr(std::uint8_t precedence) {
  // <binary_expr>  = <cast_expr> { <binary_operator> <cast_expr> }  

  // <cast_expr> OR a binaryExpr of inferior priority.
  ExprResult lhsResult;
  if (precedence > 0)
    lhsResult = parseBinaryExpr(precedence - 1);
  else
    lhsResult = parseCastExpr();

  if (!lhsResult) {
    if (!lhsResult.wasSuccessful())
      return ExprResult::Error();
    return ExprResult::NotFound();
  }

  Expr* lhs = lhsResult.get();
  BinaryExpr* rtr = nullptr;

  // { <binary_operator> <cast_expr> }  
  while (true) {
    // <binary_operator>
    auto binop_res = parseBinaryOp(precedence);
    if (!binop_res) // No operator found : break.
      break;

    // <cast_expr> OR a binaryExpr of inferior priority.
    ExprResult rhsResult;
    if (precedence > 0)
      rhsResult = parseBinaryExpr(precedence - 1);
    else
      rhsResult = parseCastExpr();


    // Handle results appropriately
		// Check for validity : we need a rhs. if we don't have one, we have an error ! 
    if (!rhsResult) {
      if(rhsResult.wasSuccessful())
        reportErrorExpected(DiagID::parser_expected_expr);
      return ExprResult::Error();
    }

    Expr* rhs = rhsResult.get();
    SourceLoc begLoc = lhs ? lhs->getRange().getBegin() : rtr->getRange().getEnd();
    SourceLoc endLoc = rhs->getRange().getEnd();

    SourceRange range(begLoc, endLoc);
    assert(range && "Invalid loc info");

    SourceRange opRange = binop_res.getRange();

    rtr = BinaryExpr::create(ctxt_, binop_res.get(), (rtr ? rtr : lhs), rhs, 
      range, opRange);
  }

  if (!rtr) {
    assert(lhs && "no rtr node + no lhs node?");
    return ExprResult(lhs);
  }
  return ExprResult(rtr);
}

Parser::ExprResult Parser::parseExpr() {
  //  <expr> = <binary_expr> [<assign_operator> <expr>] 
  auto lhs = parseBinaryExpr();
  if (!lhs)
    return lhs;

  if (auto op = parseAssignOp()) {
    auto rhs = parseExpr();
    if (!rhs) {
      if(rhs.wasSuccessful())
        reportErrorExpected(DiagID::parser_expected_expr);
      return ExprResult::Error();
    }

    SourceLoc begLoc = lhs.get()->getRange().getBegin();
    SourceLoc endLoc = rhs.get()->getRange().getEnd();

    SourceRange range(begLoc, endLoc);
    assert(range && "Invalid loc info");

    SourceRange opRange = op.getRange();
    return ExprResult(BinaryExpr::create(ctxt_, op.get(), lhs.get(), rhs.get(),
      range, opRange));
  }
  return ExprResult(lhs);
}

Parser::ExprResult Parser::parseParensExpr(SourceLoc* leftPLoc, SourceLoc* rightPLoc) {
  // <parens_expr> = '(' <expr> ')'

  // '('
  auto leftParens = consumeBracket(SignType::S_ROUND_OPEN);
  if (!leftParens)
    return ExprResult::NotFound();

  Expr* rtr = nullptr;
    
  // <expr>
  if (auto expr = parseExpr())
    rtr = expr.get();
  else  {
    // no expr, handle error & attempt to recover if it's allowed. If recovery is successful, return "not found"
    if(expr.wasSuccessful())
      reportErrorExpected(DiagID::parser_expected_expr);

    if (resyncToSign(SignType::S_ROUND_CLOSE, /* stopAtSemi */ true, /*consumeToken*/ true))
      return ExprResult::NotFound();
    else
      return ExprResult::Error();
  }

  assert(rtr && "The return value shouldn't be null!");

  // ')'
  auto rightParens = consumeBracket(SignType::S_ROUND_CLOSE);
  if (!rightParens) {
    // no ), handle error & attempt to recover 
    reportErrorExpected(DiagID::parser_expected_closing_roundbracket);

    if (!resyncToSign(SignType::S_ROUND_CLOSE, /* stopAtSemi */ true, /*consumeToken*/ false))
      return ExprResult::Error();
      
    // If we recovered successfuly, place the Sloc into rightParens
    rightParens = consumeBracket(SignType::S_ROUND_CLOSE);
  }

  // Save the locs if the caller wants it.
  if (leftPLoc) {
    assert(leftParens && "invalid loc info");
    *leftPLoc = leftParens;
  }

  if (rightPLoc) {
    assert(rightParens && "invalid loc info");
    *rightPLoc = rightParens;
  }

  // Just return the expression we parsed.
  return ExprResult(rtr);
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
      if (expr.wasSuccessful()) {
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

Parser::Result<ExprVector> Parser::parseParensExprList(SourceLoc* LParenLoc, SourceLoc *RParenLoc) {
  // <parens_expr_list>  = '(' [ <expr_list> ] ')'
  // '('
  auto leftParens = consumeBracket(SignType::S_ROUND_OPEN);
  if (!leftParens)
    return Result<ExprVector>::NotFound();

  if (LParenLoc)
    *LParenLoc = leftParens;

  ExprVector exprs;

  //  [ <expr_list> ]
  if (auto exprlist = parseExprList())
    exprs = exprlist.get();
  else if (!exprlist.wasSuccessful()) {
    // error? Try to recover from it, if success, just discard the expr list,
    // if no success return error.
    if (resyncToSign(SignType::S_ROUND_CLOSE, /* stopAtSemi */ true, /*consumeToken*/ false)) {
      SourceLoc loc = consumeBracket(SignType::S_ROUND_CLOSE);

      if (RParenLoc)
        *RParenLoc = loc;

      return Result<ExprVector>(ExprVector()); // if recovery is successful, return an empty expression list.
    }
    return Result<ExprVector>::Error();
  }

  SourceLoc rightParens = consumeBracket(SignType::S_ROUND_CLOSE);
  // ')'
  if (!rightParens) {
    reportErrorExpected(DiagID::parser_expected_closing_roundbracket);

    if (resyncToSign(SignType::S_ROUND_CLOSE, /* stopAtSemi */ true, /*consumeToken*/ false))
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

Parser::Result<BinaryExpr::OpKind> Parser::parseAssignOp() {
  using BinOp = BinaryExpr::OpKind;

  if (auto equal = consumeSign(SignType::S_EQUAL)) {
    // Try to match a S_EQUAL. If failed, that means that the next token isn't a =
    // If it succeeds, we found a '==' (=> this is the comparison operator) and
    // we must undo 
    if (!consumeSign(SignType::S_EQUAL))
      return Result<BinOp>(BinOp::Assign,SourceRange(equal));
    undo();
  }
  return Result<BinOp>::NotFound();
}

Parser::Result<UnaryExpr::OpKind> Parser::parseUnaryOp() {
  using UOp = UnaryExpr::OpKind;

  if (auto excl = consumeSign(SignType::S_EXCL_MARK))
    return Result<UOp>(UOp::LNot, SourceRange(excl));
  else if (auto minus = consumeSign(SignType::S_MINUS))
    return Result<UOp>(UOp::Minus, SourceRange(minus));
  else if (auto plus = consumeSign(SignType::S_PLUS))
    return Result<UOp>(UOp::Plus, SourceRange(plus));
  return Result<UOp>::NotFound();
}

Parser::Result<BinaryExpr::OpKind> Parser::parseBinaryOp(std::uint8_t priority) {
  using BinOp = BinaryExpr::OpKind;

  // Check current Token validity, also check if it's a sign because if it isn't we can return directly!
  if (!getCurtok().isValid() || !getCurtok().isSign())
    return Result<BinOp>::NotFound();


  switch (priority) {
    case 0: // * / %
      if (auto asterisk = consumeSign(SignType::S_ASTERISK)) {
        if (!consumeSign(SignType::S_ASTERISK)) // Disambiguation between '**' and '*'
          return Result<BinOp>(BinOp::Mul, SourceRange(asterisk));
        // undo if not found
        undo();
      }
      else if (auto slash = consumeSign(SignType::S_SLASH))
        return Result<BinOp>(BinOp::Div, SourceRange(slash));
      else if (auto percent = consumeSign(SignType::S_PERCENT))
        return Result<BinOp>(BinOp::Mod, SourceRange(percent));
      break;
    case 1: // + -
      if (auto plus = consumeSign(SignType::S_PLUS))
        return Result<BinOp>(BinOp::Add, SourceRange(plus));
      else if (auto minus = consumeSign(SignType::S_MINUS))
        return Result<BinOp>(BinOp::Sub, SourceRange(minus));
      break;
    case 2: // > >= < <=
      if (auto lessthan = consumeSign(SignType::S_LESS_THAN)) {
        if (auto equal = consumeSign(SignType::S_EQUAL))
          return Result<BinOp>(BinOp::LE, SourceRange(lessthan,equal));
        return Result<BinOp>(BinOp::LT, SourceRange(lessthan));
      }
      else if (auto grthan = consumeSign(SignType::S_GREATER_THAN)) {
        if (auto equal = consumeSign(SignType::S_EQUAL))
          return Result<BinOp>(BinOp::GE, SourceRange(grthan,equal));
        return Result<BinOp>(BinOp::GT, SourceRange(grthan));
      }
      break;
    case 3:  // == !=
      // try to match '=' twice.
      if (auto equal1 = consumeSign(SignType::S_EQUAL)) {
        if (auto equal2 = consumeSign(SignType::S_EQUAL))
          return Result<BinOp>(BinOp::Eq, SourceRange(equal1,equal2));
        // undo if not found
        undo();
      }
      else if (auto excl = consumeSign(SignType::S_EXCL_MARK)) {
        if (auto equal =consumeSign(SignType::S_EQUAL))
          return Result<BinOp>(BinOp::NEq, SourceRange(excl,equal));
        // undo if not found
        undo();
      }
      break;
    case 4: // &&
      if (auto amp1 = consumeSign(SignType::S_AMPERSAND)) {
        if (auto amp2 = consumeSign(SignType::S_AMPERSAND))
          return Result<BinOp>(BinOp::LAnd, SourceRange(amp1,amp2));
      }
      break;
    case 5: // ||
      if (auto vbar1 = consumeSign(SignType::S_VBAR)) {
        if (auto vbar2 = consumeSign(SignType::S_VBAR))
          return Result<BinOp>(BinOp::LOr, SourceRange(vbar1,vbar2));
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