
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
				E_CRITICAL("[RUNTIME] Tried to concat a node without a left_ child.")

			auto leftstr = std::get<std::string>(node->left_->accept(this)); // get left str
			std::string rightstr = "";

			if (node->right_)
				rightstr = std::get<std::string>(node->right_->accept(this));
			
			return FVal(std::string(leftstr + rightstr));
		}
		catch (std::bad_variant_access&) 
		{
			E_CRITICAL("[RUNTIME] Critical error: Attempted to concat values while one of them wasn't a string.")
		}
	}
	else if (node->op_ == parse::optype::CAST)
	{
		return ch.castTo(node->totype_ , node->left_->accept(this));
	}
	else if (node->op_ == parse::optype::PASS)
	{
		if (!node->left_)
			E_CRITICAL("[RUNTIME] Tried to pass a value to parent node, but the node did not have a left_ child.")

		return node->left_->accept(this);
	}
	else if (parse::isCondition(node->op_))
	{
		if(!node->left_ || !node->right_)
			E_CRITICAL("[RUNTIME] Attempted to run a comparison operation on a node without 2 children")
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
			E_CRITICAL("[RUNTIME] Tried to perform an operation on a node without a left_ and/or right child.")

		double dleftval = fvalToDouble(node->left_->accept(this));
		double drightval = fvalToDouble(node->right_->accept(this));
		double result = performOp(node->op_, dleftval, drightval);

		if (fitsInValue(node->totype_, result) || (node->op_ == parse::CAST)) // If the results fits or we desire to cast the result
			return ch.castTo(node->totype_, result);		// Cast to result type
		else
			return ch.castTo(fval_float, result);	// Cast to float instead to keep information from being lost.
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
		E_CRITICAL("[RUNTIME] Can't convert str to double")
		return 0.0;
	}
	E_CRITICAL("[RUNTIME] Reached end of function.Unimplemented type in FVal?")
	return 0.0;
}
bool Moonshot::RTExprVisitor::compareVal(const parse::optype & op, const FVal & l, const FVal & r)
{
	using namespace parse;
	const double lval = fvalToDouble(l);
	const double rval = fvalToDouble(r);
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
			E_CRITICAL("[RUNTIME] Defaulted. Unimplemented condition operation?")
			return false;
	}
}
double RTExprVisitor::performOp(const parse::optype& op, const double & l, const double & r)
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
			{
				E_ERROR("[RUNTIME] Division by zero.")
				return 0.0;
			}
			else 
				return l / r;
		case MOD:
			return fmod(l, r); // Modulus support floating point op
		case EXP:
			return pow(l, r); // Exponential
		default:
			E_CRITICAL("[RUNTIME] Defaulted.")
			return 0.0;
	}
}

bool Moonshot::RTExprVisitor::fitsInValue(const std::size_t& typ, const double & d)
{
	using namespace parse;
	switch (typ)
	{
		case fval_bool:
			return true; // When we want to cast to bool, we usually don't care to lose information, we just want a true/false result.
		case fval_int:
			if (d > INT_MAX || d < INT_MIN)
				return false;
			return true;
		case fval_float:
			return true;
		case fval_char:
			if (d < -127 || d > 127)
				return false;
			return true;
		case invalid_index:
			E_CRITICAL("[RUNTIME] Index was invalid")
			return false;
		default:
			E_CRITICAL("[RUNTIME] Defaulted. Unimplemented type? Or tried to convert to string?")
			return false;
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
		E_CRITICAL("[RUNTIME] Can't convert string to value")
			return { false, FVal() };
	}
	else if (b1 || b2) // Goal is a string -> error if val != string
	{
		std::stringstream ss;
		ss << "[RUNTIME] Can't convert a string to an arithmetic type and vice versa. Value:" << v << std::endl;
		E_ERROR(ss.str())
	}
	else if (std::is_same<FVal, VAL>::value)
	{
		E_ERROR("[RUNTIME] FVAL ! What are you doing here!")
	}
	else // Conversion will work. Proceed !
	{
		if constexpr(std::is_same<VAL, std::string>::value)
			E_CRITICAL("[RUNTIME] Can't convert string to value")
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
				E_CRITICAL("[RUNTIME] Failed cast");
		}
	}
	return { false,FVal(0) };
}

template<typename GOAL>
std::pair<bool, FVal> Moonshot::RTExprVisitor::castHelper::castTypeTo(const GOAL & type,double v)
{
	if constexpr(std::is_same<GOAL, std::string>::value)
	{
		E_CRITICAL("[RUNTIME] Failed cast");
		return { true,FVal() };
	}
	else	
		return { true, FVal((GOAL)v) };
}

FVal Moonshot::RTExprVisitor::castHelper::castTo(const std::size_t& goal, const FVal & val)
{
	std::pair<bool, FVal> rtr = std::make_pair<bool, FVal>(false, FVal());
	std::visit([&](const auto& a, const auto& b) {
		rtr = castTypeTo(a, b);
	},getSampleFValForIndex(goal), val);

	if (rtr.first)
		return rtr.second;
	else
		E_ERROR("[RUNTIME] Failed typecast (TODO:Show detailed error message")
	return FVal();
}

FVal Moonshot::RTExprVisitor::castHelper::castTo(const std::size_t& goal, const double & val)
{
	std::pair<bool, FVal> rtr;
	std::visit([&](const auto& a) {
		rtr = castTypeTo(a,val);
	},getSampleFValForIndex(goal));
	if (rtr.first)
		return rtr.second;
	E_ERROR("[RUNTIME] Failed typecast from double (TODO:Show detailed error message")
	return FVal();
}
