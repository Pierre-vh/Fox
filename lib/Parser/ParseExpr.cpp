//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : ParseExpr.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/AST/ASTContext.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"
#include "Fox/Lexer/Lexer.hpp"
#include "Fox/Parser/Parser.hpp"
#include "utfcpp/utf8.hpp"
#include <sstream>

using namespace fox;

Parser::Result<Expr*> Parser::parseSuffix(Expr* base) {
  assert(base && "Base cannot be nullptr!");

  // <suffix> = '.' <id> | '[' <expr> ']' | <parens_expr_list>

  // "." <id> 
  // '.'
  if (auto dotLoc = tryConsume(TokenKind::Dot).getBeginLoc()) {
    // <id>
    if (isCurTokAnIdentifier()) {
      Identifier id;
      SourceRange idRange;
      std::tie(id, idRange) = consumeIdentifier();
      return Result<Expr*>(
        UnresolvedDotExpr::create(ctxt, base , id, idRange, dotLoc)
      );
    }
    reportErrorExpected(DiagID::expected_iden);
    return Result<Expr*>::Error();
  }

  // '[' <expr> ']
  // '['
  if (SourceLoc lsquare = tryConsume(TokenKind::LSquare).getBeginLoc()) {
    // <expr>
    if (auto exprResult = parseExpr()) {
      // ']'
      SourceLoc rsquare = tryConsume(TokenKind::RSquare).getBeginLoc();
      if (!rsquare) {
        reportErrorExpected(DiagID::expected_rbracket);
        diagEngine.report(DiagID::to_match_this_bracket, lsquare);
        // Try to recover to a ']'
        if (skipUntilDeclStmtOr(TokenKind::RSquare))
          rsquare = consume().getBeginLoc();
        else
          return Result<Expr*>::Error();
      }

      return Result<Expr*>(
        SubscriptExpr::create(ctxt, base, exprResult.get(), rsquare)
      );
    }
    else if (exprResult.isNotFound())
      reportErrorExpected(DiagID::expected_expr);

    if (skipUntilDeclStmtOr(TokenKind::RSquare)) {
      consume();
      return Result<Expr*>(base);
    }

    return Result<Expr*>::Error();
  }

  // <parens_expr_list>
  SourceRange parenRange;
  if (auto exprlist = parseParensExprList(&parenRange)) {
    assert(parenRange && "parseParensExprList didn't complete the parenRange?");
    return Result<Expr*>(
      CallExpr::create(ctxt, base, exprlist.move(), parenRange)
    );
  }
  else if (exprlist.isError())
    return Result<Expr*>::Error();

  return Result<Expr*>::NotFound();
}

Parser::Result<Expr*> Parser::parseDeclRef() {
  // <decl_call> = <id> 
  if (isCurTokAnIdentifier()) {
    Identifier id;
    SourceRange idRange;
    std::tie(id, idRange) = consumeIdentifier();
    return Result<Expr*>(UnresolvedDeclRefExpr::create(ctxt, id,
                         idRange));
  }
  return Result<Expr*>::NotFound();
}

namespace {
  // Tries to convert a token to an int literal.
  // If cannot be converted to a int (because it's too large)
  FoxInt tokToIntLit(DiagnosticEngine& engine, Token tok) { 
    // <int_literal> = {(Digit 0 through 9)}
    // TODO: Use something other than stringstream to avoid conversion
    // to std::string
    std::istringstream iss(tok.str.to_string());
    FoxInt tmp;
    if (iss >> tmp) return tmp;
    engine.report(DiagID::err_while_inter_int_lit, tok.range);
    return 0;
  }

  // Tries to convert a token to an double literal.
  // If cannot be converted to a int (because it's too large)
  FoxDouble tokToDoubleLit(DiagnosticEngine& engine, Token tok) {
    // <double_literal> = <int_literal> '.' <int_literal>
    // TODO: Use something other than stringstream to avoid conversion
    // to std::string
    std::istringstream iss(tok.str.to_string());
    FoxDouble tmp;
    if (iss >> tmp) return tmp;
    engine.report(DiagID::err_while_inter_double_lit, tok.range);
    return 0.0;
  }
}

std::string Parser::normalizeString(string_view str) {
  // Remove the delimiters
  // If the string has 2 only characters, that means it's empty 
  // (there's only the delimiters)
  if (str.size() == 2) return "";
  // Else remove the delimiters
  str = str.substr(1, str.size()-2);
  // Normalize the string, converting all escape sequences to the correct
  // characters.
  std::string normalized;
  for (auto it = str.begin(), end = str.end(); it != end; ++it) {
    // <escape_seq> = '\' ('n' | 'r' | 't' | '\' | ''' | '"' | '0')
    if ((*it) == '\\') {
      // Save the loc of the backslash
      const char* backslashPtr = it;
      // Advance and save the loc of the escape character
      const char* escapeCharPtr = ++it;
      // If we have an escape character, we should always have something
      // after it. If we don't, that means that the lexer considered
      // an escaped delimiter (such as \') as the end of the lexer.
      // (and that'd be a bug)
      assert((it != end) && "unfinished escape sequences");
      switch (*it) {
        // \0 becomes 0
        case '0':
          normalized.push_back(0);
          break;
        // \n becomes LF
        case 'n':
          normalized.push_back('\n');
          break;
        // \r becomes CR
        case 'r':
          normalized.push_back('\r');
          break;
        // \t becomes TAB
        case 't':
          normalized.push_back('\t');
          break;
        // \\ becomes a single backslash
        case '\\':
          normalized.push_back('\\');
          break;
        // \' becomes '
        case '\'':
          normalized.push_back('\'');
          break;
        // \" becomes "
        case '"':
          normalized.push_back('"');
          break;
        // Else this escape sequence is not valid, diagnose and
        // ignore it.
        default:
          SourceLoc loc = lexer.getLocFromPtr(backslashPtr);
          SourceLoc extra = lexer.getLocFromPtr(escapeCharPtr);
          diagEngine
            .report(DiagID::unknown_escape_seq, loc)
            .setExtraRange(SourceRange(extra))
            .addArg(string_view(backslashPtr, 2));
          // We're still going to add the escaped character
          // without the backslash to avoid cascading errors.
          normalized.push_back(*it);
          break;
      }
    }
    else
      normalized.push_back(*it);
  }
  return normalized;
}

StringLiteralExpr*
Parser::createStringLiteralExprFromToken(const Token& tok) {
  // <string_literal> = '"' {<string_item>} '"'
  assert(tok.is(TokenKind::StringLiteral)
    && "incorrect token kind");
  assert((tok.str.size() >= 2) && (tok.str.front() == '"')
    && (tok.str.back() == '"') 
    && "ill-formed StringLiteral token");
  // Normalize the string
  std::string normalized = normalizeString(tok.str);
  string_view str;
  // If it's not empty, allocate a copy of it in the ASTContext.
  if (normalized.size()) str = ctxt.allocateCopy(normalized);
  // Create the node and return it.
  return StringLiteralExpr::create(ctxt, str, tok.range);
}

Expr* 
Parser::createCharLiteralExprFromToken(const Token& tok) {
  // <char_literal> = ''' {<char_item>} '''
  assert(tok.is(TokenKind::CharLiteral)
    && "incorrect token kind");
  assert((tok.str.size() >= 2) && (tok.str.front() == '\'')
    && (tok.str.back() == '\'') 
    && "ill-formed CharLiteral token");
  // Normalize the string
  std::string normalized = normalizeString(tok.str);
  const auto normBeg = normalized.begin();
  const auto normEnd = normalized.end();
  // Check that the size is acceptable
  std::size_t numCPs = utf8::distance(normBeg, normEnd);
  Expr* theExpr = nullptr;
  // A Char literal cannot be empty
  if (numCPs == 0) {
    diagEngine.report(DiagID::empty_char_lit, tok.range);
    theExpr = ErrorExpr::create(ctxt, tok.range);
  }
  // A Char literal cannot contain more than one codepoint
  else if (numCPs > 1) {
    diagEngine.report(DiagID::multiple_cp_in_char_lit, tok.range);
    theExpr = ErrorExpr::create(ctxt, tok.range);
  }
  // Else we're good, create a valid CharLiteralExpr.
  else {
    FoxChar theChar = static_cast<FoxChar>(utf8::peek_next(normBeg, normEnd));
    theExpr = CharLiteralExpr::create(ctxt, theChar, tok.range);
  }
  assert(theExpr && "Return Expr is nullptr");
  return theExpr;
}

Parser::Result<Expr*> Parser::parsePrimitiveLiteral() {
  // <primitive_literal>  = <bool_literal> | <int_literal> | <float_literal> |
  //                      <string_literal> | <char_literal> 
  auto tok = getCurtok();
  Expr* expr = nullptr;
  SourceRange range = tok.range;

  // <bool_literal> = "true" | "false"
  if (tok.is(TokenKind::TrueKw))
    expr = BoolLiteralExpr::create(ctxt, true, range);
  else if (tok.is(TokenKind::FalseKw))
    expr = BoolLiteralExpr::create(ctxt, false, range);
  // <string_literal> = '"' {<char_item>} '"'
  else if (tok.is(TokenKind::StringLiteral))
    expr = createStringLiteralExprFromToken(tok);
  // <char_literal> = ''' <char_item> '''
  else if (tok.is(TokenKind::CharLiteral))
    expr = createCharLiteralExprFromToken(tok);
  // <int_literal> = {(Digit 0 through 9)}
  else if (tok.is(TokenKind::IntConstant))
    expr = IntegerLiteralExpr::create(ctxt, 
                                      tokToIntLit(diagEngine, tok), range);
  // <double_literal> = <int_literal> '.' <int_literal>
  else if (tok.is(TokenKind::DoubleConstant))
    expr = DoubleLiteralExpr::create(ctxt, 
                                     tokToDoubleLit(diagEngine, tok), range);
  // Not a literal
  else
    return Result<Expr*>::NotFound();
  assert(expr && "no expr");
  consume();
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
    if (elist.isNotFound()) {
      reportErrorExpected(DiagID::expected_rbracket);
      diagEngine.report(DiagID::to_match_this_bracket, begLoc);
    }

    if (skipUntilDeclStmtOr(TokenKind::RSquare))
      endLoc = consume().getBeginLoc();
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
  else if (lit.isError())
    return Result<Expr*>::Error();

  // = <decl_call>
  if (auto declcall = parseDeclRef())
    return declcall;
  else if (declcall.isError())
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

  // <exponent_operator>  ('*' '*')
  if (auto expOp = tryConsume(TokenKind::StarStar)) {
    // <prefix_expr>
    auto rhs = parsePrefixExpr();
    if (!rhs) {
      if (rhs.isNotFound())
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
      if (prefixexpr.isNotFound())
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
      if (rhsResult.isNotFound())
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
      if (rhs.isNotFound())
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
  SourceLoc lparen = tryConsume(TokenKind::LParen).getBeginLoc();
  if (!lparen) return Result<Expr*>::NotFound();

  // <expr>
  Expr* rtr = nullptr;
  if (auto expr = parseExpr())
    rtr = expr.get();
  else  {
    if (expr.isNotFound())
      reportErrorExpected(DiagID::expected_expr);

    if (!skipUntilDeclStmtOr(TokenKind::RParen))
      return Result<Expr*>::Error();
  }
  assert(rtr && "The return value shouldn't be null!");

  // ')'
  if (!tryConsume(TokenKind::RParen)) {
    // Diagnose
    reportErrorExpected(DiagID::expected_rparen);
    diagEngine.report(DiagID::to_match_this_paren, lparen);
    // Attempt to recover
    if (skipUntilDeclStmtOr(TokenKind::RParen))
      consume();
    else
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
      // Diagnose if the expression was not found
      if (expr.isNotFound())
        reportErrorExpected(DiagID::expected_expr);
      return Result<ExprVector>::Error();
    }
  }

  return Result<ExprVector>(exprs);
}

Parser::Result<ExprVector> 
Parser::parseParensExprList(SourceRange *parenRange) {
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
    // Try to recover to our ')'
    if (!skipUntilDeclStmtOr(TokenKind::RParen))
      return Result<ExprVector>::Error();
  }

  // ')'
  SourceLoc rightParens = tryConsume(TokenKind::RParen).getBeginLoc();
  if (!rightParens) {
    reportErrorExpected(DiagID::expected_rparen);
    diagEngine.report(DiagID::to_match_this_paren, leftParens);

    if (skipUntilDeclStmtOr(TokenKind::RParen))
      rightParens = consume().getBeginLoc();
    else 
      return Result<ExprVector>::Error();
  }

  if (parenRange)
    *parenRange = SourceRange(leftParens, rightParens);

  return Result<ExprVector>(exprs);
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
  if (auto minus = tryConsume(TokenKind::Minus))
    return success(UOp::Minus, minus);
  if (auto plus = tryConsume(TokenKind::Plus))
    return success(UOp::Plus, plus);
  if (auto dollar = tryConsume(TokenKind::Dollar))
    return success(UOp::ToString, dollar);
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
      if (tryConsume(TokenKind::Slash))
        return success(BinOp::Div);
      if (tryConsume(TokenKind::Percent))
        return success(BinOp::Mod);
      break;
    case 1: // + -
      if (tryConsume(TokenKind::Plus))
        return success(BinOp::Add);
      if (tryConsume(TokenKind::Minus))
        return success(BinOp::Sub);
      break;
    case 2: // > >= < <=
      if (tryConsume(TokenKind::Less))
        return success(BinOp::LT);
      if (tryConsume(TokenKind::LessEqual))
        return success(BinOp::LE);
      if (tryConsume(TokenKind::Greater))
        return success(BinOp::GT);
      if (tryConsume(TokenKind::GreaterEqual))
        return success(BinOp::GE);
      break;
    case 3:  // == !=
      if (tryConsume(TokenKind::EqualEqual))
        return success(BinOp::Eq);
      if (tryConsume(TokenKind::ExclaimEqual))
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