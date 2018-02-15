////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : TypeCheck.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "TypeCheck.hpp"
// Include nodes
#include "Moonshot/Fox/AST/Nodes/ASTExpr.hpp" 
#include "Moonshot/Fox/AST/Nodes/ASTVarDeclStmt.hpp" 
// Other
#include "Moonshot/Common/Context/Context.hpp" // context
#include "Moonshot/Common/Exceptions/Exceptions.hpp" // exceptions
#include "Moonshot/Common/Types/Types.hpp" // Types
#include "Moonshot/Common/Types/TypesUtils.hpp" // Types Utilities
#include "Moonshot/Common/Utils/Utils.hpp"
#include <sstream> // std::stringstream

using namespace Moonshot;
using namespace TypeUtils;


TypeCheckVisitor::TypeCheckVisitor(Context& c,const bool& testmode) : context_(c), datamap_(c)
{
	if (testmode)
	{
		datamap_.declareValue(
			var::varattr("TESTVALUE", indexes::fval_int, false),
			FVal(IntType())
		);
	}
	node_ctxt_.cur_binop = binaryOperation::PASS;
	node_ctxt_.dir = directions::UNKNOWN;
}

TypeCheckVisitor::~TypeCheckVisitor()
{

}

void TypeCheckVisitor::visit(ASTBinaryExpr & node)
{
	if (!context_.isSafe()) // If an error was thrown earlier, just return. We can't check the tree if it's unhealthy (and it would be pointless anyways)
		return;

	if (node.left_ && node.right_)
	{
		if (node.op_ == binaryOperation::ASSIGN)
		{
			if (!isAssignable(node.left_.get()))
			{
				context_.reportError("Assignement operation requires a assignable data type to the left of the operator !");
				return;
			}
		}
		// VISIT BOTH CHILDREN
		// get left expr result type
		auto left = visitAndGetResult(node.left_.get(), directions::LEFT,node.op_);
		// get right expr result type
		auto right = visitAndGetResult(node.right_.get(), directions::RIGHT,node.op_);
		// SPECIAL CHECK 1: CHECK IF THIS IS A CONCAT OP,CONVERT IT 
		if (canConcat(left,right) && (node.op_ == binaryOperation::ADD))
			node.op_ = binaryOperation::CONCAT;
		// CHECK VALIDITY OF EXPRESSION
		value_ = getExprResultType(
			node.op_
			, left
			, right
		);
	}
	else
	{
		throw Exceptions::ast_malformation("Binary Expression node was incomplete, it did not have both a left and right child.");
		return;
	}

	node.resultType_ = value_;
}

void TypeCheckVisitor::visit(ASTUnaryExpr & node)
{
	if (!context_.isSafe()) // If an error was thrown earlier, just return. We can't check the tree if it's unhealthy (and it would be pointless anyways)
		return;
	if (!node.child_)
		throw Exceptions::ast_malformation("UnaryExpression node did not have any child.");

	// Get the child's return type. Don't change anything, as rtr_value is already set by the accept function.
	const auto childttype = visitAndGetResult(node.child_.get());
	// Throw an error if it's a string. Why ? Because we can't apply the unary operators LOGICNOT or NEGATIVE on a string.
	if (childttype == indexes::fval_str)
	{
		// no unary op can be performed on a string
		std::stringstream output;
		output << "Can't perform unary operation " << Util::getFromDict(kUop_dict, node.op_) << " on a string.";
		context_.reportError(output.str());
	}
	// SPECIAL CASES : (LOGICNOT) and (NEGATIVE ON BOOLEANS)
	if (node.op_ == unaryOperation::LOGICNOT)
		value_ = indexes::fval_bool; // Return type is a boolean
	else if ((node.op_ == unaryOperation::NEGATIVE) && (value_ == indexes::fval_bool)) // If the subtree returns a boolean and we apply the negate operation, it'll return a int.
		value_ = indexes::fval_int;

	node.resultType_  = value_;
}

void TypeCheckVisitor::visit(ASTCastExpr & node)
{
	if (!context_.isSafe()) // If an error was thrown earlier, just return. We can't check the tree if it's unhealthy (and it would be pointless anyways)
		return;
	if (!node.child_)
		throw Exceptions::ast_malformation("CastExpression node did not have any child.");

	const auto result = visitAndGetResult(node.child_.get());
	if (canCastTo(node.getCastGoal(), result))
		value_ = node.getCastGoal();
	else
	{
		context_.reportError("Can't perform cast : " + indexToTypeName(result) + " to " + indexToTypeName(node.getCastGoal()));
		value_ = indexes::invalid_index;
	}
}

void TypeCheckVisitor::visit(ASTLiteral & node)
{
	value_ = node.val_.index();		// Just put the value in rtr->type.
}

void TypeCheckVisitor::visit(ASTVarDeclStmt & node)
{
	// check for impossible/illegal assignements;
	if (node.initExpr_) // If the node has an initExpr.
	{
		// get the init expression type.
		const auto iexpr_type = visitAndGetResult(node.initExpr_.get());
		// check if it's possible.
		if (!canAssign(
			node.vattr_.type_,
			iexpr_type
		))
		{
			context_.reportError("Can't perform initialization of variable \"" + node.vattr_.name_ + "\". Type of initialization expression is unassignable to the desired variable type.\nFor further information, see the errors thrown earlier!");
		}
	}
	datamap_.declareValue(
		node.vattr_,
		getSampleFValForIndex(node.vattr_.type_) // Using a sample fval, so we don't need to store any "real" values in there.
	);
	// returns nothing
}

void TypeCheckVisitor::visit(ASTVarCall & node)
{
	auto searchResult = datamap_.retrieveVarAttr(node.varname_);
	if ((node_ctxt_.dir == directions::LEFT) && (node_ctxt_.cur_binop == binaryOperation::ASSIGN) && searchResult.isConst)
	{
		context_.reportError("Can't assign a value to const variable \"" + searchResult.name_ + "\"");
		value_ = indexes::invalid_index;
	}
	else 
		value_ =  searchResult.type_; // The error will be thrown by the symbols table itself if the value doesn't exist.
}

bool TypeCheckVisitor::isAssignable(const IASTExpr* op) const
{
	if (dynamic_cast<const ASTVarCall*>(op)) // if the node's a ASTVarCall, it's assignable.
		return true;
	return false;
}

bool TypeCheckVisitor::shouldOpReturnFloat(const binaryOperation & op) const
{
	// only div and exp are affected by this exception. The exponent is because with negative ones it acts like a fraction.(
	return (op == binaryOperation::DIV) || (op == binaryOperation::EXP);
}

std::size_t TypeCheckVisitor::getExprResultType(const binaryOperation& op, std::size_t& lhs, const std::size_t& rhs)
{
	if (!context_.isSafe()) // If an error was thrown earlier, just return. 
		return indexes::invalid_index;
	// first, quick, simple check : we can only verify results between 2 basic types.
	if (op == binaryOperation::ASSIGN)
	{
		if (canAssign(lhs, rhs))
			return lhs; // Assignements return the value  of the lhs.
		else
		{
			std::stringstream output;
			output << "Can't assign a " << indexToTypeName(rhs) << " to a variable of type " << indexToTypeName(lhs) << std::endl;
			context_.reportError(output.str());
			return indexes::invalid_index;
		}
	}
	else if (op == binaryOperation::CONCAT)
		return indexes::fval_str;
	else if (isBasic(lhs) && isBasic(rhs))
	{
		if (lhs == rhs) // Both sides are identical
		{
			if (isComparison(op))
			{
				if (isCompJoinOp(op) && !isArithmetic(lhs)) // If we have a compJoinOp and types aren't arithmetic : problem
				{
					context_.reportError("Operations AND (&&) and OR (||) require types convertible to boolean on each side.");
					return indexes::invalid_index;
				}
				return indexes::fval_bool; // Else, it's normal, return type's a boolean.
			}
			else if ((lhs == indexes::fval_str) && (op != binaryOperation::CONCAT)) // We have strings and it's not a concat op :
			{
				context_.reportError("Can't perform operations other than addition (concatenation) on strings");
				return indexes::invalid_index;
			}
			else if (lhs == indexes::fval_bool) // We have bools and they're not compared : the result will be an integer.
				return	indexes::fval_bool;
			else
				return shouldOpReturnFloat(op) ? indexes::fval_float : lhs; // Else, we just keep the type, unless it's a divison
		}
		else if (!isArithmetic(lhs) || !isArithmetic(rhs)) // Two different types, and one of them is a string?
		{
			context_.reportError("Can't perform an operation on a string and a numeric type."); 		// We already know the type is different (see the first if) so we can logically assume that we have a string with a numeric type. Error!
			return indexes::invalid_index;
		}
		else if (isComparison(op)) // Comparing 2 arithmetic types ? return type's a boolean
			return indexes::fval_bool;
		else
		{
			if (shouldOpReturnFloat(op))
				return indexes::fval_float; // if op = division Or exp, return type's a float, because they may return floats under certain circumstances
			else
				return getBiggest(lhs, rhs); // Else, it's just a normal operation, and the return type is the one of the "biggest" of the 2 sides
		}
		return indexes::invalid_index;
	}
	else // One of the types is a non-basic type.
	{
		context_.reportError("Can't typecheck an expression where lhs,rhs or both sides aren't basic types (int/char/bool/string/float).");
		return indexes::invalid_index;
	}
	throw std::logic_error("getExprResultType() Defaulted.");
	// return indexes::invalid_index;
}