
/************************************************************
Author : Pierre van Houtryve
Contact :
e-mail : pierre.vanhoutryve@gmail.com

Description : See header

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

#include "RTExprVisitor.h"

using namespace Moonshot;

RTExprVisitor::RTExprVisitor()
{
}


RTExprVisitor::~RTExprVisitor()
{
}

FVal Moonshot::RTExprVisitor::visit(ASTExpr * node)
{
	castHelper ch;
	if (node->op_ == parse::optype::CONCAT)
	{
		try
		{
			if (!node->left_)
				E_CRITICAL("Tried to concat a node without a left_ child.")

			auto leftstr = std::get<std::string>(node->left_->accept(this)); // get left str
			std::string rightstr = "";

			if (node->right_)
				rightstr = std::get<std::string>(node->right_->accept(this));
			
			return FVal(std::string(leftstr + rightstr));
		}
		catch (std::bad_variant_access&) 
		{
			E_CRITICAL("Critical error: Attempted to concat values while one of them wasn't a string.")
		}
	}
	else if (node->op_ == parse::optype::CAST)
	{
		return ch.castTo(node->totype_ , node->left_->accept(this));
	}
	else if (node->op_ == parse::optype::PASS)
	{
		if (!node->left_)
			E_CRITICAL("Tried to pass a value to parent node, but the node did not have a left_ child.")

		return node->left_->accept(this);
	}
	else if (parse::isCondition(node->op_))
	{
		if(!node->left_ || !node->right_)
			E_CRITICAL("Attempted to run a comparison operation on a node without 2 children")
		double dleftval = fvalToDouble(node->left_->accept(this));
		double drightval = fvalToDouble(node->right_->accept(this));
		return FVal(compareVal(
			node->op_,
			node->left_->accept(this),
			node->right_->accept(this)
		)
		);
	}
	else if (node->op_ == parse::LOGICNOT || node->op_ == parse::NEGATE)
	{
		double lval = fvalToDouble(node->left_->accept(this));
		if (node->op_ == parse::LOGICNOT)
		{
			return FVal(
				lval == 0 // If the value differs equals zero, return true
			);
		}
		else
		{
			lval = -lval; // Negate the number
			return ch.castTo(node->totype_, lval);		// Cast to result type
		}
	}
	else
	{
		if (!node->left_ || !node->right_)
			E_CRITICAL("Tried to perform an operation on a node without a left_ and/or right child.")

		double dleftval = fvalToDouble(node->left_->accept(this));
		double drightval = fvalToDouble(node->right_->accept(this));
		double result = performOp(node->op_, dleftval, drightval);
		return ch.castTo(node->totype_, result);		// Cast to result type
	}
	return FVal();
}

FVal Moonshot::RTExprVisitor::visit(ASTValue * node)
{
	return node->val_;
}

double Moonshot::RTExprVisitor::fvalToDouble(const FVal & fval)
{
	if (std::holds_alternative<int>(fval))
		return (double)std::get<int>(fval);
	else if (std::holds_alternative<float>(fval))
		return (double)std::get<float>(fval);
	else if (std::holds_alternative<char>(fval))
		return (double)std::get<char>(fval);
	else if (std::holds_alternative<bool>(fval))
		return (double)std::get<bool>(fval);
	else if (std::holds_alternative<std::string>(fval))
	{
		E_CRITICAL("Can't convert str to double")
		return 0.0;
	}
}
bool Moonshot::RTExprVisitor::compareVal(const parse::optype & op, const FVal & l, const FVal & r)
{
	using namespace parse;
	double lval = fvalToDouble(l);
	double rval = fvalToDouble(r);
	switch (op)
	{
		case AND:
			return (lval != 0) && (rval != 0);
		case OR:
			return (lval != 0) || (rval != 0);
		case LESS_OR_EQUAL:
			return lval <= rval;
		case GREATER_OR_EQUAL:
			return lval >= rval;
		case LESS_THAN:
			return lval < rval;
		case GREATER_THAN:
			return lval > rval;
		case EQUAL:
			return lval == rval;
		case NOTEQUAL:
			return lval != rval;
		default:
			E_CRITICAL("Defaulted.")
			return false;
	}
}
double Moonshot::RTExprVisitor::performOp(const parse::optype& op, const double & l, const double & r)
{
	using namespace parse;
	switch (op)
	{
		case ADD:
			return l + r;
		case MINUS:
			return l - r;
		case MUL:
			return l * r;
		case DIV:
			if(r == 0)
				E_ERROR("Division by zero.")
			return l / r;
		case MOD:
			return fmod(l, r); // Modulus support floating point op
		case EXP:
			return pow(l, r); // Exponential
		default:
			E_CRITICAL("Defaulted.")
			return 0.0;
	}
}

template<typename GOAL, typename VAL, bool b1, bool b2>
std::pair<bool, FVal> RTExprVisitor::castHelper::castTypeTo(const GOAL& type,VAL v)
{
	if constexpr(std::is_same<GOAL, VAL>::value) // Direct conversion
		return { true , FVal(v) };
	else if (b1 && b2)
	{
		return { true, FVal(v) };
	}
	else if (std::is_same<VAL, std::string>::value)
	{
		E_CRITICAL("Can't convert string to value")
			return { false, FVal() };
	}
	else if (b1 || b2) // Goal is a string -> error if val != string
	{
		std::stringstream ss;
		ss << "Can't convert a string to an arithmetic type and vice versa. Value:" << v << std::endl;
		E_ERROR(ss.str())
	}
	else if (std::is_same<FVal, VAL>::value)
	{
		E_ERROR("Fval!")
	}
	else // Conversion will work. Proceed !
	{
		if constexpr(std::is_same<VAL, std::string>::value)
			E_CRITICAL("Can't convert string to value")
		else
		{
			if constexpr (std::is_same<int, GOAL>::value)
				return { true,FVal((int)v) };
			else if (std::is_same<float, GOAL>::value)
				return { true,FVal((float)v) };
			else if (std::is_same<char, GOAL>::value)
				return { true,FVal((char)v) };
			else if (std::is_same<bool, GOAL>::value)
				return { true,FVal((bool)v) };
			else
				E_CRITICAL("Failed cast");
		}
	}
	return { false,FVal(0) };
}

template<typename GOAL>
std::pair<bool, FVal> Moonshot::RTExprVisitor::castHelper::castTypeTo(const GOAL & type,double v)
{
	if constexpr(std::is_same<GOAL, std::string>::value)
	{
		E_CRITICAL("Failed cast");
		return { true,FVal() };
	}
	else	
		return { true, FVal((GOAL)v) };
}

FVal Moonshot::RTExprVisitor::castHelper::castTo(const parse::types & goal, const FVal & val)
{
	std::pair<bool, FVal> rtr = std::make_pair<bool, FVal>(false, FVal());
	std::visit([&](const auto& a, const auto& b) {
		rtr = castTypeTo(a, b);
	}, parseTypes_toFVal(goal), val);

	if (rtr.first)
		return rtr.second;
	else
		E_ERROR("Failed typecast (TODO:Show detailed error message")
	return FVal();
}

FVal Moonshot::RTExprVisitor::castHelper::castTo(const parse::types & goal, const double & val)
{
	std::pair<bool, FVal> rtr;
	std::visit([&](const auto& a) {
		rtr = castTypeTo(a,val);
	}, parseTypes_toFVal(goal));
	if (rtr.first)
		return rtr.second;
	E_ERROR("Failed typecast from double (TODO:Show detailed error message")
	return FVal();
}
