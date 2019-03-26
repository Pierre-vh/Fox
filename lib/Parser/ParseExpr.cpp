//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : ParseExpr.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/AST/ASTContext.hpp"
#include "Fox/Parser/Parser.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"
#include "Fox/Common/StringManipulator.hpp"
#include <sstream>

using namespace fox;

Parser::Result<Expr*> Parser::parseSuffix(Expr* base) {
  assert(base && "Base cannot be nullptr!");

  // <suffix> = '.' <id> | '[' <expr> ']' | <parens_expr_list>

  SourceLoc endLoc;

  // "." <id> 
  // '.'
  if (auto dotLoc = tryConsume(TokenKind::Dot).getBeginLoc()) {
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
      reportErrorExpected(DiagID::expected_iden);
      return Result<Expr*>::Error();
    }
  }
  // '[' <expr> ']
  // '['
  else if (tryConsume(TokenKind::LSquare)) {
    // <expr>
    if (auto expr = parseExpr()) {
      // ']'
      SourceLoc rsquare = tryConsume(TokenKind::RSquare).getBeginLoc();
      if (!rsquare) {
        reportErrorExpected(DiagID::expected_closing_square_bracket);

        if (resyncTo(TokenKind::RSquare, /* stopAtSemi */ true, 
          /*consumeToken*/ false))
          rsquare = tryConsume(TokenKind::RSquare).getBeginLoc();
        else
          return Result<Expr*>::Error();
      }


      return Result<Expr*>(
        ArraySubscriptExpr::create(ctxt, base, expr.get(), rsquare)
      );
    }
    else {
      if (expr.isNotFound())
        reportErrorExpected(DiagID::expected_expr);

      // Resync. if Resync is successful, return the base as the result 
      // (don't alter it) to fake a success
      // , if it's not, return an Error.
      if (resyncTo(TokenKind::RSquare, /* stopAtSemi */ true, /*consume*/ true))
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

namespace {
  bool tokToBoolLit(Token tok) {
    // <bool_literal> = "true" | "false"
    string_view str = tok.str;
    if(str == "true") return true;
    if(str == "false") return false;
    fox_unreachable("unhandled bool literal token string");
  }

  string_view tokToStringLit(Token tok) {
    // <string_literal> = '"' {<char_item>} '"'
    // FIXME: Normalize string literals here.
    string_view str = tok.str;
    assert((str.front() == '"') && (str.back() == '"') 
      && "unhandled string literal token string");
    return str.substr(1, str.size()-2);
  }

  FoxChar tokToCharLit(Token tok) {
    // <char_literal> = ''' <char_item> '''
    // FIXME: Normalize char literal here, handle empty char literals
    // and the ones that are too long.
    string_view str = tok.str;
    assert((str.front() == '\'') && (str.back() == '\'') 
      && "unhandled char literal token string");
    StringManipulator manip(str);
    return manip.getChar(1);
  }

  // Tries to convert a token to an int literal.
  // If cannot be converted to a int (because it's too large)
  FoxInt tokToIntLit(DiagnosticEngine& engine, Token tok) { 
    // <int_literal> = {(Digit 0 through 9)}
    // TODO: Use something other than stringstream to avoid conversion
    // to std::string
    std::istringstream iss(tok.str.to_string());
    FoxInt tmp;
    if(iss >> tmp) return tmp;
    engine.report(DiagID::err_while_inter_int_lit, tok.range);
    return 0;
  }

  // Tries to convert a token to an double literal.
  // If cannot be converted to a int (because it's too large)
  FoxInt tokToDoubleLit(DiagnosticEngine& engine, Token tok) {
    // <double_literal> = <int_literal> '.' <int_literal>
    // TODO: Use something other than stringstream to avoid conversion
    // to std::string
    std::istringstream iss(tok.str.to_string());
    FoxDouble tmp;
    if(iss >> tmp) return tmp;
    engine.report(DiagID::err_while_inter_double_lit, tok.range);
    return 0.0;
  }
}

Parser::Result<Expr*> Parser::parsePrimitiveLiteral() {
  // <primitive_literal>  = One literal of the following type : Integer,
  //                        Floating-point, Boolean, String, Char
  auto tok = getCurtok();
  Expr* expr = nullptr;
  SourceRange range = tok.range;

  // <bool_literal> = "true" | "false"
  if (tok.is(TokenKind::BoolLiteral))
    expr = BoolLiteralExpr::create(ctxt, tokToBoolLit(tok), range);
  // <string_literal> = '"' {<char_item>} '"'
  else if (tok.is(TokenKind::DoubleQuoteText)) {
    // The token class has already allocated of the string in the ASTContext,
    // so it's safe to use the string_view given by getStringValue
    expr = StringLiteralExpr::create(ctxt, tokToStringLit(tok), range);
  }
  // <char_literal> = ''' <char_item> '''
  else if (tok.is(TokenKind::SingleQuoteText))
    expr = CharLiteralExpr::create(ctxt, tokToCharLit(tok), range);
  // <int_literal> = {(Digit 0 through 9)}
  else if (tok.is(TokenKind::IntLiteral))
    expr = IntegerLiteralExpr::create(ctxt, 
                                      tokToIntLit(diagEngine, tok), range);
  // <double_literal> = <int_literal> '.' <int_literal>
  else if (tok.is(TokenKind::DoubleLiteral))
    expr = DoubleLiteralExpr::create(ctxt, 
                                     tokToDoubleLit(diagEngine, tok), range);
  // Not a literal
  else
    return Result<Expr*>::NotFound();
  assert(expr && "no expr");
  next();
  return Result<Expr*>(expr);
}

Parser::Result<Expr*> Parser::parseArrayLiteral() {
  // <array_literal>  = '[' [<expr_list>] ']'
  auto begLoc = tryConsume(TokenKind::LSquare).getBeginLoc();
  if (!begLoc)
    return Result<Expr*>::NotFound();
  
  // [<expr_list>]
  auto elist = parseExprList(); 

  // We don't check for errors because even if it failed, the Result object
  // will construct a empty ExprList for us!

  // ']'
  SourceLoc endLoc = tryConsume(TokenKind::RSquare).getBeginLoc();
  if (!endLoc) {
    if (elist.isNotFound())
      reportErrorExpected(DiagID::expected_closing_square_bracket);

    if (resyncTo(TokenKind::RSquare, /* stopAtSemi */ true, 
      /*consumeToken*/ false))
      endLoc = tryConsume(TokenKind::RSquare).getBeginLoc();
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
        reportErrorExpected(DiagID::expected_expr);
        
      return Result<Expr*>::Error();
    }

    return Result<Expr*>(BinaryExpr::create(ctxt, BinaryExpr::OpKind::Pow,
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
        reportErrorExpected(DiagID::expected_expr);

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
  if (tryConsume(TokenKind::AsKw)) {
    // <type>
    if (auto tyRes = parseType()) {
      TypeLoc tl = tyRes.get();
      SourceLoc begLoc = prefixexpr.get()->getBeginLoc();
      SourceLoc endLoc = tl.getEndLoc();

      SourceRange range(begLoc, endLoc);
      assert(range && "Invalid loc info");

      return Result<Expr*>(CastExpr::create(ctxt, tl, prefixexpr.get()));
    }
    else {
      reportErrorExpected(DiagID::expected_type);
      return Result<Expr*>::Error();
    }
  }

  return prefixexpr;
}

Parser::Result<Expr*> Parser::parseBinaryExpr(unsigned precedence) {
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
        reportErrorExpected(DiagID::expected_expr);
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
        reportErrorExpected(DiagID::expected_expr);
      return Result<Expr*>::Error();
    }

    return Result<Expr*>(BinaryExpr::create(ctxt, op.get(), 
      lhs.get(), rhs.get(), opRange));
  }
  return lhs;
}

Parser::Result<Expr*> Parser::parseParensExpr() {
  // <parens_expr> = '(' <expr> ')'

  // '('
  if (!tryConsume(TokenKind::LParen)) 
    return Result<Expr*>::NotFound();

  // <expr>
  Expr* rtr = nullptr;
  if (auto expr = parseExpr())
    rtr = expr.get();
  else  {
    // no expr, handle error & attempt to recover if it's allowed. 
    // If recovery is successful, return "not found"
    if(expr.isNotFound())
      reportErrorExpected(DiagID::expected_expr);

    if (resyncTo(TokenKind::RParen, /* stopAtSemi */ true, /*consume*/ true))
      return Result<Expr*>::NotFound();
    else
      return Result<Expr*>::Error();
  }

  assert(rtr && "The return value shouldn't be null!");

  // ')'
  if (!tryConsume(TokenKind::RParen)) {
    // no ), handle error & attempt to recover 
    reportErrorExpected(DiagID::expected_closing_round_bracket);

    if (!resyncTo(TokenKind::RParen, /* stopAtSemi */ true, /*consume*/ false))
      return Result<Expr*>::Error();
  }

  return Result<Expr*>(rtr);
}

Parser::Result<ExprVector> Parser::parseExprList() {
  // <expr_list> = <expr> {',' <expr> }
  auto firstExpr = parseExpr();
  if (!firstExpr)
    return Result<ExprVector>::NotFound();

  ExprVector exprs;
  exprs.push_back(firstExpr.get());
  while (tryConsume(TokenKind::Comma)) {
    if (auto expr = parseExpr())
      exprs.push_back(expr.get());
    else {
      if (expr.isNotFound()) {
        // if the expression was just not found, revert the comma consuming and
        // let the caller deal with the extra comma after the expression list.
        undo();
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
  auto leftParens = tryConsume(TokenKind::LParen).getBeginLoc();
  if (!leftParens)
    return Result<ExprVector>::NotFound();

  ExprVector exprs;

  //  [ <expr_list> ]
  if (auto exprlist = parseExprList())
    exprs = exprlist.get();
  else if (exprlist.isError()) {
    // error? Try to recover from it, if success, just discard the expr list,
    // if no success return error.
    if (resyncTo(TokenKind::RParen, /*stopAtSemi*/ true, /*consume*/ false)) {
      SourceLoc loc = tryConsume(TokenKind::RParen).getBeginLoc();

      if (RParenLoc)
        *RParenLoc = loc;

        // if recovery is successful, return an empty expression list.
      return Result<ExprVector>(ExprVector());
    }
    return Result<ExprVector>::Error();
  }

  SourceLoc rightParens = tryConsume(TokenKind::RParen).getBeginLoc();
  // ')'
  if (!rightParens) {
    reportErrorExpected(DiagID::expected_closing_round_bracket);

    if (resyncTo(TokenKind::RParen, /* stopAtSemi */ true, 
      /*consumeToken*/ false))
      rightParens = tryConsume(TokenKind::RParen).getBeginLoc();
    else 
      return Result<ExprVector>::Error();
  }

  if (RParenLoc)
    *RParenLoc = rightParens;

  return Result<ExprVector>(exprs);
}

SourceRange Parser::parseExponentOp() {
  if (getCurtok().is(TokenKind::StarStar)) {
    SourceRange range = getCurtok().range;
    next();
    return range;
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

  if (auto equal = tryConsume(TokenKind::Equal)) 
    return success(BinOp::Assign, equal);
  return Result<BinOp>::NotFound();
}

Parser::Result<UnaryExpr::OpKind> 
Parser::parseUnaryOp(SourceRange& range) {
  using UOp = UnaryExpr::OpKind;

  auto success = [&](UOp op, SourceRange opRange) {
    range = opRange;
    return Result<UOp>(op);
  };

  if (auto excl = tryConsume(TokenKind::Exclaim))
    return success(UOp::LNot, excl);
  else if (auto minus = tryConsume(TokenKind::Minus))
    return success(UOp::Minus, minus);
  else if (auto plus = tryConsume(TokenKind::Plus))
    return success(UOp::Plus, plus);
  return Result<UOp>::NotFound();
}

Parser::Result<BinaryExpr::OpKind> 
Parser::parseBinaryOp(unsigned priority, SourceRange& range) {
  using BinOp = BinaryExpr::OpKind;

  Token cur = getCurtok();

  auto success = [&](BinOp op) {
    range = cur.range;
    return Result<BinOp>(op);
  };

  switch (priority) {
    case 0: // * / %
      if (tryConsume(TokenKind::Star))
        return success(BinOp::Mul);
      else if (tryConsume(TokenKind::Slash))
        return success(BinOp::Div);
      else if (tryConsume(TokenKind::Percent))
        return success(BinOp::Mod);
      break;
    case 1: // + -
      if (tryConsume(TokenKind::Plus))
        return success(BinOp::Add);
      else if (tryConsume(TokenKind::Minus))
        return success(BinOp::Sub);
      break;
    case 2: // > >= < <=
      if (tryConsume(TokenKind::Less))
        return success(BinOp::LT);
      else if (tryConsume(TokenKind::LessEqual))
        return success(BinOp::LE);
      else if (tryConsume(TokenKind::Greater))
        return success(BinOp::GT);
      else if (tryConsume(TokenKind::GreaterEqual))
        return success(BinOp::GE);
      break;
    case 3:  // == !=
      if (tryConsume(TokenKind::EqualEqual))
        return success(BinOp::Eq);
      else if (tryConsume(TokenKind::ExclaimEqual))
        return success(BinOp::NEq);
      break;
    case 4: // &&
      if (tryConsume(TokenKind::AmpAmp))
        return success(BinOp::LAnd);
      break;
    case 5: // ||
      if (tryConsume(TokenKind::PipePipe))
        return success(BinOp::LOr);
      break;
    default:
      fox_unreachable("Unknown priority");
  }
  return Result<BinOp>::NotFound();
}