////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : TypeCheck.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "TypeCheck.h"

using namespace Moonshot;
using namespace fv_util;

TypeCheck::TypeCheck(const bool& testmode)
{
	if(testmode)
	symtable_.declareValue(
		var::varattr("TESTVALUE", fval_int, false),
		FVal(0)
	);
}

TypeCheck::~TypeCheck()
{

}

void TypeCheck::visit(ASTExpr & node)
{
	if (!E_CHECKSTATE) // If an error was thrown earlier, just return. We can't check the tree if it's unhealthy (and it would be pointless anyways)
		return;
	//// SET CUROP
	curop_ = node.op_;
	//////////////////////////////////
	/////NODES WITH 2 CHILDREN////////
	//////////////////////////////////
	if (node.left_ && node.right_) 
	{
		// VISIT BOTH CHILDREN
		// get left expr result type
		auto left = visitAndGetResult(node.left_,parse::direction::LEFT);
		// get right expr result type
		auto right = visitAndGetResult(node.right_, parse::direction::RIGHT);
		// SPECIAL CHECK 1: CHECK IF THIS IS A CONCAT OP,CONVERT IT 
		if (fval_traits<std::string>::isEqualTo(left) && fval_traits<std::string>::isEqualTo(right)  && (node.op_ == parse::ADD))
			node.op_ = parse::CONCAT;
		// CHECK VALIDITY OF EXPRESSION
		rtr_type_ = getExprResultType(
				node.op_
			,	left		
			,	right
			);
	}
	/////////////////////////////////////////
	/////NODES WITH ONLY A LEFT CHILD////////
	/////////////////////////////////////////
	else if(node.left_)// We only have a left node
	{
		// CAST NODES
		if (node.op_ == parse::CAST) // check validity of cast
		{
			auto result = visitAndGetResult(node.left_);
			if (!canCastTo(node.totype_, result))
			{
				E_ERROR("Can't perform cast : " + indexToTypeName(result) + " to " + indexToTypeName(node.totype_));
				rtr_type_ = invalid_index;
			}
			else
				rtr_type_ = node.totype_;
		}
		// UNARY OPS
		else if (parse::isUnary(node.op_))
		{
			// We have a unary operation
			// Get left's return type. Don't change anything, as rtr_value is already set by the accept function.
			auto lefttype = visitAndGetResult(node.left_);
			// Throw an error if it's a string. Why ? Because we can't apply the unary operators LOGICNOT or NEGATE on a string.
			if(fval_traits<std::string>::isEqualTo(lefttype))
			{
				std::stringstream output;
				output << "Can't perform unary operation " << getFromDict(parse::kOptype_dict, node.op_) << " on a string.";
				E_ERROR(output.str());
			}
			// SPECIAL CASES : (LOGICNOT)(NEGATE ON BOOLEANS)
			if (node.op_ == parse::LOGICNOT)
				rtr_type_ = fval_bool; // Return type is a boolean
			else if ((node.op_ == parse::NEGATE) && fval_traits<bool>::isEqualTo(rtr_type_)) // If the subtree returns a boolean and we apply the negate operation, it'll return a int.
				rtr_type_ = fval_int;
		}
		else
			E_CRITICAL("A Node only had a left_ child, and wasn't a unary op.");
	}
	//////////////////////////////////
	/////ERROR CASES//////////////////
	//////////////////////////////////
	else
	{
		// Okay, this is far-fetched, but can be possible if our parser is broken. It's better to check this here :
		// getting in this branch means that we only have a right_ node.
		E_CRITICAL("Node was in an invalid state.");
	}
	node.totype_ = rtr_type_;

	if (!isBasic(node.totype_) && E_CHECKSTATE)
	{
		if (node.totype_ == invalid_index)
			E_ERROR("Type was invalid.");
		else 
			E_CRITICAL("node.totype was not a basic type.");
	}
}

void TypeCheck::visit(ASTRawValue & node)
{
	rtr_type_ = node.val_.index();		// Just put the value in rtr->type.
}

void TypeCheck::visit(ASTVarDeclStmt & node)
{
	// check for impossible/illegal assignements;
	if (node.initExpr_) // If the node has an initExpr.
	{
		// get the init expression type.
		auto iexpr_type = visitAndGetResult(node.initExpr_);
		// check if it's possible.
		if (!canAssign(
			node.vattr_.type,
			iexpr_type
		))
		{
			E_ERROR("Can't perform initialization of variable \"" + node.vattr_.name + "\". Type of initialization expression is unassignable to the desired variable type.\nFor further information, see the errors thrown earlier!");
		}
	}
	symtable_.declareValue(
		node.vattr_,
		getSampleFValForIndex(node.vattr_.type) // Using a sample fval, so we don't need to store any "real" values in there.
	);
	// returns nothing
}

void TypeCheck::visit(ASTVarCall & node)
{
	auto searchResult = symtable_.retrieveVarAttr(node.varname_);
	if ((curdir_ == parse::direction::LEFT) && (curop_ == parse::optype::ASSIGN) && searchResult.isConst)
	{
		E_ERROR("Can't assign a value to const variable \"" + searchResult.name + "\"");
		rtr_type_ = invalid_index;
	}
	else 
		rtr_type_ =  searchResult.type; // The error will be thrown by the symbols table itself if the value doesn't exist.
}

std::size_t TypeCheck::getExprResultType(const parse::optype& op, std::size_t& lhs, const std::size_t& rhs)
{
	if (!E_CHECKSTATE) // If an error was thrown earlier, just return. 
		return invalid_index;
	// first, quick, simple check : we can only verify results between 2 basic types.
	if (op == parse::ASSIGN)
	{
		if (canAssign(lhs, rhs))
			return lhs; // Assignements return the value  of the lhs.
		else
		{
			std::stringstream output;
			output << "Can't assign a " << indexToTypeName(rhs) << " to a variable of type " << indexToTypeName(lhs) << std::endl;
			E_ERROR(output.str());
			return invalid_index;
		}
	}
	else if (isBasic(lhs) && isBasic(rhs))
	{
		if (lhs == rhs) // Both sides are identical
		{
			if (parse::isComparison(op))
			{
				if (parse::isCompJoinOp(op) && !isArithmetic(lhs)) // If we have a compJoinOp and types aren't arithmetic : problem
				{
					E_ERROR("Operations AND (&&) and OR (||) require types convertible to boolean on each side.");
					return invalid_index;
				}
				return fval_bool; // Else, it's normal, return type's a boolean.
			}
			else if (fval_traits<std::string>::isEqualTo(lhs) && (op != parse::CONCAT)) // We have strings and it's not a concat op :
			{
				E_ERROR("Can't perform operations other than addition (concatenation) on strings");
				return invalid_index;
			}
			else if (fval_traits<bool>::isEqualTo(lhs)) // We have bools and they're not compared : the result will be an integer.
				return	fval_bool;
			else
				return (op == parse::DIV) ? fval_float : lhs; // Else, we just keep the type, unless it's a divison
		}
		else if (!isArithmetic(lhs) || !isArithmetic(rhs)) // Two different types, and one of them is a string?
		{
			E_ERROR("Can't perform an operation on a string and a numeric type."); 		// We already know the type is different (see the first if) so we can logically assume that we have a string with a numeric type. Error!
			return invalid_index;
		}
		else if (parse::isComparison(op)) // Comparing 2 arithmetic types ? return type's a boolean
			return fval_bool;
		else
		{
			if (op == parse::DIV)
				return fval_float; // if op = division, return type's a float.
			else
				return getBiggest(lhs, rhs); // Else, it's just a normal operation, and the return type is the one of the "biggest" of the 2 sides
		}
		return invalid_index;
	}
	else // One of the types is a non-basic type.
	{
		E_ERROR("Can't typecheck an expression where lhs,rhs or both sides aren't basic types (int/char/bool/string/float).");
		return invalid_index;
	}
	E_CRITICAL("getExprResultType() Defaulted.");
	return invalid_index;
}