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

bool Parser::matchID()
{
	token t = getToken();
	if (t.type == lex::TT_IDENTIFIER)
	{
		pos_ += 1;
		return true;
	}
	return false;
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

std::pair<bool, std::size_t> Moonshot::Parser::matchTypeKw()
{
	token t = getToken();
	if (t.type == lex::TT_KEYWORD)
	{
		std::size_t rtr = invalid_index;
		switch (t.kw_type)
		{			
			case lex::T_INT:
				rtr = fval_int;
				break;
			case lex::T_FLOAT:
				rtr = fval_float;
				break;
			case lex::T_CHAR:
				rtr = fval_char;
				break;
			case lex::T_STRING:
				rtr = fval_str;
				break;
			case lex::T_BOOL:
				rtr = fval_bool;
				break;
		}
		if (rtr != invalid_index)
		{
			pos_ += 1;
			return { true,rtr };
		}
	}
	return { false, invalid_index };
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
	output << " Unexpected token " << getToken().showFormattedTokenData() << std::endl;
	E_ERROR(output.str());
}

void Parser::errorExpected(const std::string & s)
{
	std::stringstream output;
	output << s << " after token " << getToken().showFormattedTokenData() << std::endl;
	E_ERROR(output.str());
}
