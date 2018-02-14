////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : RTExprVisitor.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "RTExprVisitor.hpp"

// Context & exceptions
#include "Moonshot/Common/Context/Context.hpp"
#include "Moonshot/Common/Exceptions/Exceptions.hpp"
// Utils, types, typecast
#include "Moonshot/Common/Utils/Utils.hpp"
#include "Moonshot/Common/Types/FVTypeTraits.hpp"
#include "Moonshot/Common/Types/TypeCast.hpp"
#include "Moonshot/Common/Types/TypesUtils.hpp"
// DataMap
#include "Moonshot/Common/DataMap/DataMap.hpp"
// Nodes needed
#include "Moonshot/Fox/AST/Nodes/ASTExpr.hpp"
#include "Moonshot/Fox/AST/Nodes/ASTVarDeclStmt.hpp"
// math operations
#include <cmath>		
#include <sstream>
// string manip
#include "Moonshot/Common/UTF8/StringManipulator.hpp"

using namespace Moonshot;
using namespace TypeUtils;

RTExprVisitor::RTExprVisitor(Context& c) : context_(c)
{
}

RTExprVisitor::RTExprVisitor(Context& c, std::shared_ptr<DataMap> symtab) : context_(c), symtab_(symtab)
{
}

RTExprVisitor::~RTExprVisitor()
{
}

void RTExprVisitor::visit(ASTBinaryExpr & node)
{
	if (!context_.isSafe())
	{
		value_ = FVal(); // return directly if errors, don't waste time evaluating "sick" nodes.
		return;
	}

	if (node.op_ == binaryOperation::PASS)
	{
		if (!node.left_)
			throw Exceptions::ast_malformation("Tried to pass a value to parent node, but the node did not have a left_ child.");
		else
		{
			node.left_->accept(*this);
			return;
		}
	}
	else if (node.op_ == binaryOperation::CONCAT)
	{
		if (node.left_ && node.right_)
		{
			auto leftval = visitAndGetResult(node.left_.get(), *this);
			auto rightval = visitAndGetResult(node.right_.get(), *this);

			value_ = concat(leftval, rightval);
			return;
		}
		else
		{
			throw Exceptions::ast_malformation("Tried to concat a node without a left_ or right  child.");
		}
	}
	else if (node.op_ == binaryOperation::ASSIGN)
	{
		if (isDataMapAvailable())
		{
			auto left_res = visitAndGetResult(node.left_.get(), *this);
			auto right_res = visitAndGetResult(node.right_.get(), *this);
			if (std::holds_alternative<var::varRef>(left_res) && isValue(right_res.index()))
			{
				symtab_->setValue(
					std::get<var::varRef>(left_res).getName(),
					right_res
				);
				value_ = right_res; // Assignement returns the value on the left !
				return;
			}
			else
				context_.reportError("Impossible assignement between " + dumpFVal(left_res) + " and " + dumpFVal(right_res));
		}
		else
			context_.logMessage("Can't perform assignement operations when the symbols table is unavailable.");
	}
	else if (node.op_ == binaryOperation::ASSIGN)
	{
		if (isDataMapAvailable())
		{
			auto vattr = visitAndGetResult(node.left_.get(), *this);
			if (std::holds_alternative<var::varRef>(vattr))
			{
				// Perform assignement
				std::string vname = std::get<var::varRef>(vattr).getName();
				node.right_->accept(*this);
				symtab_->setValue(vname, value_);
			}
			else
				// this could use context_.error instead. On the long run, if this error happens often and malformed code is the source, use context_.error instead of a throw
				throw Exceptions::ast_malformation("Can't assign a value if lhs isn't a variable !");
		}
		else
			context_.logMessage("Can't perform assignement operations when the symbols table isn't available.");
	}
	else if (isComparison(node.op_))
	{
		if (!node.left_ || !node.right_)
			throw Exceptions::ast_malformation("Attempted to run a comparison operation on a node without 2 children");
		
		const FVal lfval = visitAndGetResult(node.left_.get(), *this);
		const FVal rfval = visitAndGetResult(node.right_.get(), *this);
		/*
			todo, deref the variables HERE, and remove the deref part of the functions _derefFirst.
		*/
		if (std::holds_alternative<std::string>(lfval) && std::holds_alternative<std::string>(rfval)) // lhs/rhs str?
		{
			if (lfval.index() == rfval.index()) // if so, lhs/rhs must both be strings to compare them.
			{
				value_ = FVal(
					compareStr(
						node.op_,
						std::get<std::string>(lfval),
						std::get<std::string>(rfval)
					)
				);
				return;
			}
			else
			{
				context_.reportError("Attempted to compare a string with an arithmetic type.");
				value_ = FVal();
				return;
			}
		}
		else if (std::holds_alternative<CharType>(lfval) && std::holds_alternative<CharType>(rfval))
		{
			value_ = FVal(compareChar(
				node.op_,
				std::get<CharType>(lfval),
				std::get<CharType>(rfval)
			));
			return;
		}
		else 
		{
			//std::cout << "Compare: Converted lhs :" << dleftval << " converted rhs: " << drightval << std::endl;
			value_ = FVal(compareVal(
				node.op_,
				lfval,
				rfval
			)
			);
			return;
		}
	}
	else
	{
		auto left_res = visitAndGetResult(node.left_.get(), *this);
		auto right_res = visitAndGetResult(node.right_.get(), *this);
		// Check if we have a string somewhere.
		if (std::holds_alternative<std::string>(left_res) || std::holds_alternative<std::string>(right_res))
		{
			context_.reportError("Can't perform an arithmetic operation on a string and an arithmetic type.");
		}
		else
		{
			const double dleftval = fvalToDouble_withDeref(left_res);
			const double drightval = fvalToDouble_withDeref(right_res);
			//std::cout << "Op: " << util::enumAsInt(node.op_) << ",Converted lhs :" << dleftval << " converted rhs: " << drightval << std::endl;
			const double result = performOp(node.op_, dleftval, drightval);

			if (fitsInValue(node.resultType_, result)) // If the results fits or we desire to cast the result
				value_ = CastUtilities::castTo(context_, node.resultType_, result);		// Cast to result type
			else
				value_ = CastUtilities::castTo(context_, indexes::fval_float, result);	// Cast to float instead to keep information from being lost.
		}
		return;
	}
	// default return
	value_ = FVal();
}

void RTExprVisitor::visit(ASTUnaryExpr & node)
{
	if (!context_.isSafe())
	{
		value_ = FVal(); // return directly if errors, don't waste time evaluating "sick" nodes.
		return;
	}

	double lval = fvalToDouble_withDeref(visitAndGetResult(node.child_.get(), *this));
	// op == loginot
	if (node.op_ == unaryOperation::LOGICNOT)
	{
		value_ = FVal(
			(bool)(lval == 0) // If the value differs equals zero, return true
		);
		return;
	}
	else if (node.op_ == unaryOperation::NEGATIVE)
		lval = -lval; // Negate the number
	else if (node.op_ == unaryOperation::POSITIVE)
	{
		/*
			For now, unary Positive is a nop.
		*/
	}

	value_ = CastUtilities::castTo(context_, node.resultType_, lval);		// Cast to result type
	return;
}

void RTExprVisitor::visit(ASTCastExpr & node)
{
	if (!context_.isSafe())
	{
		value_ = FVal(); // return directly if errors, don't waste time evaluating "sick" nodes.
		return;
	}
	value_ = castTo_withDeref(node.getCastGoal(), visitAndGetResult(node.child_.get(), *this));
	return;
}

void RTExprVisitor::visit(ASTLiteral & node)
{
	value_ = node.val_;
	return;
}

void RTExprVisitor::visit(ASTVarCall & node)
{
	if (isDataMapAvailable())
	{
		value_ = symtab_->retrieveVarAttr(node.varname_).createRef(); // this returns a reference, because it's needed for assignement operations.
		return;
	}
	else
	{
		context_.logMessage("Can't retrieve values if the symbols table is not available.");
		value_ = FVal();
		return;
	}
}

void RTExprVisitor::setDataMap(std::shared_ptr<DataMap> symtab)
{
	symtab_ = symtab;
}

FVal RTExprVisitor::getResult() const
{
	return value_;
}

double RTExprVisitor::fvalToDouble_withDeref(FVal fval)
{
	// If fval is a reference, dereference it first
	if (std::holds_alternative<var::varRef>(fval))
	{
		if (isDataMapAvailable())
		{
			fval = symtab_->retrieveValue(
				std::get<var::varRef>(fval).getName()
			);
		}
		else
			context_.logMessage("Can't dereference variable if the symbols table is not available.");
	}
	if (!isBasic(fval.index()))
	{
		std::stringstream out;
		out << "Can't perform conversion to double on a non basic type.(FVal index:" << fval.index() << ")\n";
		out << dumpFVal(fval);
		context_.reportFatalError(out.str());
	}
	else if (!isArithmetic(fval.index()))
	{
		std::stringstream out;
		out << "Can't perform conversion to double on a non arithmetic type. (FVal index:" << fval.index() << ")\n";
		out << dumpFVal(fval);
		context_.reportFatalError(out.str());
	}
	else if (std::holds_alternative<IntType>(fval))
		return (double)std::get<IntType>(fval);
	else if (std::holds_alternative<float>(fval))
		return (double)std::get<float>(fval);
	else if (std::holds_alternative<CharType>(fval))
		return (double)std::get<CharType>(fval);
	else if (std::holds_alternative<bool>(fval))
		return (double)std::get<bool>(fval);
	else
		throw std::logic_error("Reached end of function.Unimplemented type in FVal?");
	return 0.0;
}
bool RTExprVisitor::compareVal(const binaryOperation & op, const FVal & l, const FVal & r)
{
	
	const double lval = fvalToDouble_withDeref(l);
	const double rval = fvalToDouble_withDeref(r);
	switch (op)
	{
		case binaryOperation::AND:
			return (lval != 0) && (rval != 0);
		case binaryOperation::OR:
			return (lval != 0) || (rval != 0);
		case binaryOperation::LESS_OR_EQUAL:
			return lval <= rval;
		case binaryOperation::GREATER_OR_EQUAL:
			return lval >= rval;
		case binaryOperation::LESS_THAN:
			return lval < rval;
		case binaryOperation::GREATER_THAN:
			return lval > rval;
		case binaryOperation::EQUAL:
			return lval == rval;
		case binaryOperation::NOTEQUAL:
			return lval != rval;
		default:
			throw std::logic_error("Defaulted. Unimplemented condition operation?");
	}
}
bool RTExprVisitor::compareStr(const binaryOperation & op, const std::string & lhs, const std::string & rhs)
{
	
	switch (op)
	{
		case binaryOperation::EQUAL:			return lhs == rhs;
		case binaryOperation::NOTEQUAL:			return lhs != rhs;
		case binaryOperation::LESS_THAN:		return lhs < rhs;
		case binaryOperation::GREATER_THAN:		return lhs > rhs;
		case binaryOperation::LESS_OR_EQUAL:	return lhs <= rhs;
		case binaryOperation::GREATER_OR_EQUAL:	return lhs > rhs;
		default:								throw std::logic_error("Operation was not a condition.");
	}
}
bool RTExprVisitor::compareChar(const binaryOperation & op, const CharType & lhs, const CharType & rhs)
{
	switch (op)
	{
		case binaryOperation::AND:
			return (lhs != 0) && (rhs != 0);
		case binaryOperation::OR:
			return (lhs != 0) || (rhs != 0);
		case binaryOperation::LESS_OR_EQUAL:
			return lhs <= rhs;
		case binaryOperation::GREATER_OR_EQUAL:
			return lhs >= rhs;
		case binaryOperation::LESS_THAN:
			return lhs < rhs;
		case binaryOperation::GREATER_THAN:
			return lhs > rhs;
		case binaryOperation::EQUAL:
			return lhs == rhs;
		case binaryOperation::NOTEQUAL:
			return lhs != rhs;
		default:
			throw std::logic_error("Defaulted. Unimplemented condition operation?");
	}
}
FVal RTExprVisitor::concat(const FVal & lhs, const FVal & rhs)
{
	std::string rtr = "";
	// lhs
	if (std::holds_alternative<std::string>(lhs))
		rtr += std::get<std::string>(lhs);
	else if (std::holds_alternative<CharType>(lhs))
		UTF8::append(rtr, std::get<CharType>(lhs));
	else
		throw std::logic_error("Invalid arguments to concat operations");

	if (std::holds_alternative<std::string>(rhs))
		rtr += std::get<std::string>(rhs);
	else if (std::holds_alternative<CharType>(rhs))
		UTF8::append(rtr, std::get<CharType>(rhs));
	else
		throw std::logic_error("Invalid arguments to concat operations");

	return FVal(rtr);
}
double RTExprVisitor::performOp(const binaryOperation& op,double l,double r)
{
	switch (op)
	{
		case binaryOperation::ADD:	return l + r;
		case binaryOperation::MINUS:	return l - r;
		case binaryOperation::MUL:	return l * r;
		case binaryOperation::DIV:
			if(r == 0)
			{
				context_.reportError("Division by zero.");
				return 0.0;
			}
			else 
				return l / r;
		case binaryOperation::MOD:
			// if the divisor is greater, it goes zero times in l, so we can directly return l
			//std::cout << "l:" << l << " r:" << r << std::endl;
			if (l > r)
			{
				const auto res = std::fmod(l, r);
				if (res < 0)
					return res + r;
				return res;
			}
			else
				return (l < 0) ? l + r : l;
			// About the res < 0 -> res + r 
			// and  (l < 0) ? l + r : l;
			// parts, it's a tip from https://stackoverflow.com/a/12277233/3232822
			// Thanks !
		case binaryOperation::EXP:
			// if exp < 0 perform 1/base**exp
			if (r < 0)
				return performOp(binaryOperation::DIV, 1, std::pow(l, -r));
			else if (r == 0)
				return 1; // Any number with exponent 0 equals 1, except 0
			else
				return std::pow(l, r);
		default:	throw std::logic_error("Can't evaluate op.");
	}
}

bool RTExprVisitor::fitsInValue(const std::size_t& typ, const double & d)
{
	switch (typ)
	{
		case indexes::fval_bool:
			return true; // When we want to cast to bool, we usually don't care to lose information, we just want a true/false result.
		case indexes::fval_int:
			if (d < limits::IntType_MIN || d > limits::IntType_MAX )
				return false;
			return true;
		case indexes::fval_float:
			return true;
		case indexes::fval_char:
			if (d < limits::CharType_MIN || d > limits::CharType_MAX)
				return false;
			return true;
		case indexes::invalid_index:
		    throw std::logic_error("Index was invalid");
		default:
			if (!isBasic(typ))
				throw std::logic_error("Can't make a \"fitInValue\" check on a non-basic type.");
			else
				throw std::logic_error("Switch defaulted. Unimplemented type?");
	}
}

bool RTExprVisitor::isDataMapAvailable() const
{
	return (symtab_ ? true : false);
}

FVal RTExprVisitor::castTo_withDeref(const std::size_t & goal, FVal val)
{
	if (std::holds_alternative<var::varRef>(val))
	{
		auto ref = std::get<var::varRef>(val);
		val = symtab_->retrieveValue(ref.getName());
	}
	else if (!isBasic(goal))
		throw std::logic_error("The Goal type was not a basic type.");

	return CastUtilities::castTo(context_,goal, val);
}
