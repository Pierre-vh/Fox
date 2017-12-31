#include "Parser.h"

using namespace Moonshot;
using namespace fv_util;

Parser::Parser(const std::shared_ptr<Lexer>& l)
{
	lex_ = l;
}

Parser::~Parser()
{
}

std::unique_ptr<ASTExpr> Parser::oneUpNode(std::unique_ptr<ASTExpr>& node, const parse::optype & op)
{
	auto newnode = std::make_unique<ASTExpr>(op);
	newnode->makeChild(parse::LEFT, node);
	return newnode;
}

std::pair<bool, token> Parser::matchValue()
{
	token t = getToken();
	if (t.type == lex::TT_VALUE)
	{
		pos_ += 1;
		return { true,t };
	}
	return { false,token() };
}



std::pair<bool, std::string> Parser::matchID()
{
	token t = getToken();
	if (t.type == lex::TT_IDENTIFIER)
	{
		pos_ += 1;
		return { true, t.str };
	}
	return { false, "" };
}

bool Parser::matchSign(const lex::signs & s)
{
	token t = getToken();
	if (t.type == lex::TT_SIGN && t.sign_type == s)
	{
		pos_ += 1;
		return true;
	}
	return false;
}

bool Parser::matchKeyword(const lex::keywords & k)
{
	token t = getToken();
	if (t.type == lex::TT_KEYWORD && t.kw_type == k)
	{
		pos_ += 1;
		return true;
	}
	return false;
}

bool Parser::matchEOI()
{
	return matchSign(lex::P_SEMICOLON);
}

std::size_t Moonshot::Parser::matchTypeKw()
{
	token t = getToken();
	pos_ += 1;
	if (t.type == lex::TT_KEYWORD)
	{
		switch (t.kw_type)
		{
			case lex::T_INT:	return fval_int;
			case lex::T_FLOAT:	return fval_float;
			case lex::T_CHAR:	return fval_char;
			case lex::T_STRING:	return fval_str;
			case lex::T_BOOL:	return fval_bool;
		}
	}
	pos_ -= 1;
	return invalid_index;
}

token Parser::getToken() const
{
	return getToken(pos_);
}

token Parser::getToken(const size_t & d) const
{
	if (d < lex_->resultSize())
		return lex_->getToken(d);
	else
		return token();
}

void Parser::errorUnexpected()
{
	E_SET_ERROR_CONTEXT("PARSING");

	std::stringstream output;
	output << "Unexpected token " << getToken().showFormattedTokenData() << std::endl;
	E_ERROR(output.str());

	E_RESET_ERROR_CONTEXT;
}

void Parser::errorExpected(const std::string & s)
{
	E_SET_ERROR_CONTEXT("PARSING");

	std::stringstream output;
	output << s << "\n[after token " << getToken(pos_-1).showFormattedTokenData() << "]" << std::endl;
	E_ERROR(output.str());

	E_RESET_ERROR_CONTEXT;
	
}
