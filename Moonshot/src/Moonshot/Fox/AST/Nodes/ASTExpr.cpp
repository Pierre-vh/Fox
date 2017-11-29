
/************************************************************
Author : Pierre van Houtryve
Contact :
e-mail : pierre.vanhoutryve@gmail.com

Description : See Header

*************************************************************
MIT License

Copyright (c) 2017 Pierre van Houtryve

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*************************************************************/

#include "ASTExpr.h"

using namespace Moonshot;

ASTExpr::ASTExpr()
{

}

ASTExpr::ASTExpr(const parse::optype & opt) : op_(opt)
{

}

ASTExpr::~ASTExpr()
{

}

void ASTExpr::showTree()
{

	std::cout << "OPERATON : " << op_ << std::endl;
	if (left_)
	{
		std::cout << "->LEFT : ";
		left_->showTree();
	}
	else
		std::cout << "NO LEFT NODE" << std::endl;

	if (right_)
	{
		std::cout << "->RIGHT : ";
		right_->showTree();
	}
	else
		std::cout << "NO RIGHT NODE" << std::endl;
}

void ASTExpr::makeChild(const parse::direction & d, std::unique_ptr<ASTExpr> &node)
{
	if (d == parse::direction::LEFT)
		left_ = std::move(node);
	else if (d == parse::direction::RIGHT)
		right_ = std::move(node);
}

void Moonshot::ASTExpr::setOpType(const parse::optype & nop)
{
	op_ = nop;
}

bool Moonshot::ASTExpr::hasNode(const parse::direction & d) const
{
	if (((d == parse::LEFT) && left_) || ((d == parse::RIGHT) && right_))
		return true;
	return false;
}

std::unique_ptr<ASTExpr> Moonshot::ASTExpr::getSimple()
{
	if (left_ && !right_ && (op_ == parse::optype::PASS))		// If the right node is empty
	{
		auto ret = std::move(left_);
		return ret;
	}
	return std::unique_ptr<ASTExpr>(nullptr);
}

parse::optype Moonshot::ASTExpr::getOpType() const
{
	return op_;
}

FVal ASTExpr::accept(IVisitor *vis)
{
	VISIT_THIS
	return FVal();
}


void ASTExpr::setMustCast(const parse::types &casttype)
{
	totype_ = casttype;
}

parse::types ASTExpr::getToType() const
{
	return totype_;
}

ASTValue::ASTValue()
{

}

ASTValue::ASTValue(const token & t)
{
	str = t.str;
	try
	{
		if (t.val_type == lex::VAL_STRING)
			val_ = t.str;
		else if (t.val_type == lex::VAL_CHAR)
			val_ = (char)t.str[0];
		else if (t.val_type == lex::VAL_BOOL)
			val_ = std::get<bool>(t.vals);
		else if (t.val_type == lex::VAL_INTEGER)
			val_ = std::get<int>(t.vals);
		else if (t.val_type == lex::VAL_FLOAT)
			val_ = std::get<float>(t.vals);
	}
	catch (const std::bad_variant_access &err)
	{
		E_CRITICAL("Tried to access a value in a variant that did not exists. ")
			std::cerr << err.what() << std::endl;
	}
}

void ASTValue::showTree()
{
	std::cout << "VALUE [" << str << "]" << std::endl;
}


FVal ASTValue::accept(IVisitor * vis)
{
	VISIT_THIS
	return FVal();
}

ASTValue::~ASTValue()
{

}


