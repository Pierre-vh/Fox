#include "RTExprVisitor.h"

using namespace Moonshot;
using namespace fv_util;

RTExprVisitor::RTExprVisitor()
{
}


RTExprVisitor::~RTExprVisitor()
{
}

FVal RTExprVisitor::visit(ASTExpr & node)
{
	if (!E_CHECKSTATE) return FVal(); // return directly if errors, don't waste time evaluating "sick" nodes.

	castHelper ch;
	if (node.op_ == parse::optype::CONCAT)
	{
		try
		{
			if (!node.left_)
				E_CRITICAL("[RUNTIME] Tried to concat a node without a left_ child.");

			auto leftstr = std::get<std::string>(node.left_->accept(*this)); // get left str
			std::string rightstr = "";

			if (node.right_)
				rightstr = std::get<std::string>(node.right_->accept(*this));
			
			return FVal(std::string(leftstr + rightstr));
		}
		catch (std::bad_variant_access&) 
		{
			E_CRITICAL("[RUNTIME] Critical error: Attempted to concat values while one of them wasn't a string.");
		}
	}
	else if (node.op_ == parse::optype::CAST)
		return ch.castTo(node.totype_ , node.left_->accept(*this));
	else if (node.op_ == parse::optype::PASS)
	{
		if (!node.left_)
			E_CRITICAL("[RUNTIME] Tried to pass a value to parent node, but the node did not have a left_ child.");

		return node.left_->accept(*this);
	}
	else if (node.op_ == parse::optype::ASSIGN)
	{
		std::cout << "/!\\/!\\ASSIGNEMENT OPERATOR IS NOT YET IMPLEMENTED : " << __FILE__ << " @ line " << __LINE__ << std::endl;
	}
	else if (parse::isComparison(node.op_))
	{
		if (!node.left_ || !node.right_)
			E_CRITICAL("[RUNTIME] Attempted to run a comparison operation on a node without 2 children");

		const FVal lfval = node.left_->accept(*this);
		const FVal rfval = node.right_->accept(*this);
		if (std::holds_alternative<std::string>(lfval) || std::holds_alternative<std::string>(rfval)) // is lhs/rhs a str?
		{
			if (lfval.index() == rfval.index()) // if so, lhs/rhs must both be strings to compare them.
			{
				try {	// Safety' check
					return FVal(
						compareStr(
							node.op_,
							std::get<std::string>(lfval),
							std::get<std::string>(rfval)
						)
					);
				}
				catch (std::bad_variant_access&)
				{
					E_CRITICAL("[RUNTIME] Critical error: Attempted to compare values while one of them wasn't a string.");
				}
			}
			else
			{
				E_ERROR("[RUNTIME] Attempted to compare a string with an arithmetic type.");
				return FVal();
			}
		}

		double dleftval = fvalToDouble(node.left_->accept(*this));
		double drightval = fvalToDouble(node.right_->accept(*this));
		return FVal(compareVal(
			node.op_,
			node.left_->accept(*this),
			node.right_->accept(*this)
		)
		);
	}
	else if (node.op_ == parse::LOGICNOT || node.op_ == parse::NEGATE)
	{
		double lval = fvalToDouble(node.left_->accept(*this));
		if (node.op_ == parse::LOGICNOT)
		{
			return FVal(
				lval == 0 // If the value differs equals zero, return true
			);
		}
		else
		{
			lval = -lval; // Negate the number
			return ch.castTo(node.totype_, lval);		// Cast to result type
		}
	}
	else
	{
		if (!node.left_ || !node.right_)
			E_CRITICAL("[RUNTIME] Tried to perform an operation on a node without a left_ and/or right child.");

		const double dleftval = fvalToDouble(node.left_->accept(*this));
		const double drightval = fvalToDouble(node.right_->accept(*this));
		const double result = performOp(node.op_, dleftval, drightval);

		if (fitsInValue(node.totype_, result) || (node.op_ == parse::CAST)) // If the results fits or we desire to cast the result
			return ch.castTo(node.totype_, result);		// Cast to result type
		else
			return ch.castTo(fval_float, result);	// Cast to float instead to keep information from being lost.
	}
	return FVal();
}

FVal RTExprVisitor::visit(ASTValue & node)
{
	return node.val_;
}

double RTExprVisitor::fvalToDouble(const FVal & fval)
{
	if (!isBasic(fval.index()))
		E_ERROR("[RUNTIME] Can't perform conversion to double on a non basic type.");
	else if (!isArithmetic(fval.index()))
		E_ERROR("[RUNTIME] Can't perform conversion to double on a non arithmetic type.");
	else if (std::holds_alternative<int>(fval))
		return (double)std::get<int>(fval);
	else if (std::holds_alternative<float>(fval))
		return (double)std::get<float>(fval);
	else if (std::holds_alternative<char>(fval))
		return (double)std::get<char>(fval);
	else if (std::holds_alternative<bool>(fval))
		return (double)std::get<bool>(fval);
	else
		E_CRITICAL("[RUNTIME] Reached end of function.Unimplemented type in FVal?");
	return 0.0;
}
bool RTExprVisitor::compareVal(const parse::optype & op, const FVal & l, const FVal & r)
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
			E_CRITICAL("[RUNTIME] Defaulted. Unimplemented condition operation?");
			return false;
	}
}
bool RTExprVisitor::compareStr(const parse::optype & op, const std::string & lhs, const std::string & rhs)
{
	using namespace parse;
	switch (op)
	{
		case EQUAL:				return lhs == rhs;
		case NOTEQUAL:			return lhs != rhs;
		case LESS_THAN:			return lhs < rhs;
		case GREATER_THAN:		return lhs > rhs;
		case LESS_OR_EQUAL:		return lhs <= rhs;
		case GREATER_OR_EQUAL:	return lhs > rhs;
		default:				E_CRITICAL("[RUNTIME] Operation was not a condition.");
			return false;
	}
}
double RTExprVisitor::performOp(const parse::optype& op,double l,double r)
{
	using namespace parse;
	switch (op)
	{
		case ADD:	return l + r;
		case MINUS:	return l - r;
		case MUL:	return l * r;
		case DIV:
			if(r == 0)
			{
				E_ERROR("[RUNTIME] Division by zero.");
				return 0.0;
			}
			else 
				return l / r;
		case MOD:	return std::fmod(l, r);
		case EXP:	return std::pow(l, r);		
		default:	E_CRITICAL("[RUNTIME] Defaulted.");
			return 0.0;
	}
}

bool RTExprVisitor::fitsInValue(const std::size_t& typ, const double & d)
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
			E_CRITICAL("[RUNTIME] Index was invalid");
			return false;
		default:
			if (!isBasic(typ))
				E_CRITICAL("[RUNTIME] Can't make a \"fitInValue\" check on a non-basic type.");
			else
				E_CRITICAL("[RUNTIME] Switch defaulted. Unimplemented type?");
			return false;
	}
}

template<typename GOAL, typename VAL, bool isGOALstr, bool isVALstr>
std::pair<bool, FVal> RTExprVisitor::castHelper::castTypeTo(const GOAL& type,VAL v)
{
	if constexpr (!fval_traits<GOAL>::isBasic || !fval_traits<VAL>::isBasic)
		E_CRITICAL("[RUNTIME] Can't cast a basic type to a nonbasic type and vice versa.");
	else if constexpr((std::is_same<GOAL, VAL>::value) || (isGOALstr && isVALstr)) // Direct conversion
		return { true , FVal(v) };
	else if constexpr (isGOALstr != isVALstr) // One of them is a string and the other isn't.
	{
		std::stringstream output;
		output << "[RUNTIME] Can't convert a string to an arithmetic type and vice versa. Value:" << v << std::endl;
		E_ERROR(output.str());
		return { false, FVal() };
	}
	else // Conversion might work. Proceed !
	{
		if constexpr (std::is_same<int, GOAL>::value)
			return { true,FVal((int)v) };
		else if constexpr (std::is_same<float, GOAL>::value)
			return { true,FVal((float)v) };
		else if constexpr (std::is_same<char, GOAL>::value)
			return { true,FVal((char)v) };
		else if constexpr (std::is_same<bool, GOAL>::value)
			return { true,FVal((bool)v) };
		else
			E_CRITICAL("[RUNTIME] Failed cast");
	}
	return { false,FVal() };
}

template<typename GOAL>
std::pair<bool, FVal> RTExprVisitor::castHelper::castTypeTo(const GOAL & type,double v)
{
	if constexpr(std::is_same<GOAL, std::string>::value)
	{
		E_CRITICAL("[RUNTIME] Failed cast - Attempted to cast to string.");
		return { true,FVal() };
	}
	else if constexpr (fval_traits<GOAL>::isBasic) // Can only attempt to convert basic types.
		return { true, FVal((GOAL)v) };
	else
		E_CRITICAL("[RUNTIME] castTypeTo defaulted. Unimplemented type?");
	return { false,FVal() };
}

FVal RTExprVisitor::castHelper::castTo(const std::size_t& goal, const FVal & val)
{
	std::pair<bool, FVal> rtr = std::make_pair<bool, FVal>(false, FVal());
	std::visit(
	[&](const auto& a, const auto& b)
	{
		rtr = castTypeTo(a, b);
	},
		getSampleFValForIndex(goal), val
	);

	if (rtr.first)
		return rtr.second;
	else
		E_ERROR("[RUNTIME] Failed typecast (TODO:Show detailed error message");
	return FVal();
}

FVal RTExprVisitor::castHelper::castTo(const std::size_t& goal, const double & val)
{
	std::pair<bool, FVal> rtr;
	std::visit(
	[&](const auto& a)
	{
		rtr = castTypeTo(a,val);
	},
		getSampleFValForIndex(goal)
	);
	if (rtr.first)
		return rtr.second;
	E_ERROR("[RUNTIME] Failed typecast from double (TODO:Show detailed error message");
	return FVal();
}
