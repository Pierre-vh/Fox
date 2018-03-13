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
#include "Moonshot/Fox/AST/Nodes/ASTDecl.hpp"
// Other
#include "Moonshot/Common/Context/Context.hpp" // context
#include "Moonshot/Common/Exceptions/Exceptions.hpp" // exceptions
#include "Moonshot/Common/Types/Types.hpp" // Types
#include "Moonshot/Common/Types/TypesUtils.hpp" // Types Utilities
#include "Moonshot/Common/Types/FoxValueUtils.hpp" // FoxValue Utils
#include "Moonshot/Common/Utils/Utils.hpp"
#include <sstream> // std::stringstream

using namespace Moonshot;
using namespace TypeUtils;


TypeCheckVisitor::TypeCheckVisitor(Context& c,const bool& testmode) : context_(c), datamap_(c)
{
	if (testmode)
	{
		datamap_.declareValue(
			FoxVariableAttr("TESTVALUE",FoxType(TypeIndex::basic_Int,true /*this is constant*/)),
			FoxValue(IntType())
		);
	}
	node_ctxt_.cur_binop = binaryOperator::DEFAULT;
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
		if (node.op_ == binaryOperator::ASSIGN_BASIC)
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
		if (canConcat(left, right) && (node.op_ == binaryOperator::ADD))
		{
			node.op_ = binaryOperator::CONCAT;
		}
		else
		{
			std::stringstream ss;
			ss << "Can't use operator " << static_cast<int>(node.op_) << " with " << left.getTypeName() << " and " << right.getTypeName() << std::endl;
			context_.logMessage(ss.str());
		}
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
	if (childttype == TypeIndex::basic_String)
	{
		// no unary op can be performed on a string
		std::stringstream output;
		output << "Can't perform unary operation " << Util::getFromDict(Dicts::kUnaryOpToStr_dict, node.op_) << " on a string.";
		context_.reportError(output.str());
	}
	// SPECIAL CASES : (LOGICNOT) and (NEGATIVE ON BOOLEANS)
	if (node.op_ == unaryOperator::LOGICNOT)
		value_ = TypeIndex::basic_Bool; // Return type is a boolean
	else if ((node.op_ == unaryOperator::NEGATIVE) && (value_ == TypeIndex::basic_Bool)) // If the subtree returns a boolean and we apply the negate operation, it'll return a int.
		value_ = TypeIndex::basic_Int;

	node.resultType_  = value_;
}

void TypeCheckVisitor::visit(ASTCastExpr & node)
{
	if (!context_.isSafe()) // If an error was thrown earlier, just return. We can't check the tree if it's unhealthy (and it would be pointless anyways)
		return;
	if (!node.child_)
		throw Exceptions::ast_malformation("CastExpression node did not have any child.");

	const auto result = visitAndGetResult(node.child_.get());
	if (canExplicitelyCastTo(node.getCastGoal(), result))
		value_ = node.getCastGoal();
	else
	{
		context_.reportError("Can't perform cast : " + result.getTypeName() + " to " + node.getCastGoal().getTypeName());
		value_ = TypeIndex::InvalidIndex;
	}
}

void TypeCheckVisitor::visit(ASTLiteralExpr & node)
{
	value_ = node.val_.index();		// Just put the value in rtr->type.
}

void TypeCheckVisitor::visit(ASTVarDecl & node)
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
		FValUtils::getSampleFValForIndex(node.vattr_.type_.getTypeIndex()) // Using a sample fval, so we don't need to store any "real" values in there.
	);
	// returns nothing
}

void TypeCheckVisitor::visit(ASTDeclRefExpr & node)
{
	auto searchResult = datamap_.retrieveVarAttr(node.declname_);
	if ((node_ctxt_.dir == directions::LEFT) && (node_ctxt_.cur_binop == binaryOperator::ASSIGN_BASIC) && searchResult.type_.isConst())
	{
		context_.reportError("Can't assign a value to const variable \"" + searchResult.name_ + "\"");
		value_ = TypeIndex::InvalidIndex;
	}
	else 
		value_ =  searchResult.type_; // The error will be thrown by the symbols table itself if the value doesn't exist.
}

bool TypeCheckVisitor::isAssignable(const IASTExpr* op) const
{
	if (dynamic_cast<const ASTDeclRefExpr*>(op)) // if the node's a ASTDeclRefExpr, it's assignable (in our naive debug version, it is!)
		return true;
	return false;
}

bool TypeCheckVisitor::shouldOpReturnFloat(const binaryOperator & op) const
{
	// only div and exp are affected by this exception. The exponent is because with negative ones it acts like a fraction.(
	return (op == binaryOperator::DIV) || (op == binaryOperator::EXP);
}

FoxType TypeCheckVisitor::getExprResultType(const binaryOperator& op, FoxType& lhs, FoxType& rhs)
{
	if (!context_.isSafe()) // If an error was thrown earlier, just return. 
		return TypeIndex::InvalidIndex;
	// first, quick, simple check : we can only verify results between 2 basic types.
	if (op == binaryOperator::ASSIGN_BASIC)
	{
		if (canAssign(lhs, rhs))
			return lhs; // Assignements return the value  of the lhs.
		else
		{
			std::stringstream output;
			output << "Can't assign a " << rhs.getTypeName() << " to a variable of type " << lhs.getTypeName() << std::endl;
			context_.reportError(output.str());
			return TypeIndex::InvalidIndex;
		}
	}
	else if (op == binaryOperator::CONCAT)
		return TypeIndex::basic_String;
	else if (lhs.isBasic() && rhs.isBasic())
	{
		if (lhs == rhs) // Both sides are identical
		{
			if (isComparison(op))
			{
				if (((op == binaryOperator::LOGIC_AND) || (op == binaryOperator::LOGIC_OR)) 
					&& !lhs.isArithmetic()) // If we have a and/or and types aren't arithmetic : problem
				{
					context_.reportError("Operations AND (&&) and OR (||) require types convertible to boolean on each side.");
					return TypeIndex::InvalidIndex;
				}
				return TypeIndex::basic_Bool; // Else, it's normal, return type's a boolean.
			}
			else if ((lhs == TypeIndex::basic_String) && (op != binaryOperator::CONCAT)) // We have strings and it's not a concat op :
			{
				context_.reportError("Can't perform operations other than concatenation on strings");
				return TypeIndex::InvalidIndex;
			}
			else if (lhs == TypeIndex::basic_Bool) // We have bools and they're not compared : the result will be an integer.
				return	TypeIndex::basic_Bool;
			else
				return shouldOpReturnFloat(op) ? TypeIndex::basic_Float : lhs; // Else, we just keep the type, unless it's a divison
		}
		else if (!lhs.isArithmetic() || !rhs.isArithmetic()) // Two different types, and one of them is a string?
		{
			context_.reportError("Can't perform an operation on a string and a numeric type."); 		// We already know the type is different (see the first if) so we can logically assume that we have a string with a numeric type. Error!
			return TypeIndex::InvalidIndex;
		}
		else if (isComparison(op)) // Comparing 2 arithmetic types ? return type's a boolean
			return TypeIndex::basic_Bool;
		else
		{
			if (shouldOpReturnFloat(op))
				return TypeIndex::basic_Float; // if op = division Or exp, return type's a float, because they may return floats under certain circumstances
			else
				return getBiggestType(lhs, rhs); // Else, it's just a normal operation, and the return type is the one of the "biggest" of the 2 sides
		}
		return TypeIndex::InvalidIndex;
	}
	else // One of the types is a non-basic type.
	{
		context_.reportError("Can't typecheck an expression where lhs,rhs or both sides aren't basic types (int/char/bool/string/float).");
		return TypeIndex::InvalidIndex;
	}
	throw std::logic_error("getExprResultType() Defaulted.");
	// return TypeIndex::InvalidIndex;
}