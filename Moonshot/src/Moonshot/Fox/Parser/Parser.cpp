#include "Parser.h"

using namespace Moonshot;
using namespace fv_util;

Parser::Parser(Lexer *l) : lex_(l)
{

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

bool Parser::matchValue(const lex::values & v)
{
	token t = getToken();
	if(t.type == lex::TT_VALUE && t.val_type == v)
	{
		pos_ += 1;
		return true;
	}
	return false;
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
	std::stringstream output;
	output << "Unexpected token " << getToken().showFormattedTokenData() << std::endl;
	E_ERROR(output.str());
}

void Parser::errorExpected(const std::string & s)
{
	std::stringstream output;
	output << s << "\n[after token " << getToken().showFormattedTokenData() << "]" << std::endl;
	E_ERROR(output.str());
}
