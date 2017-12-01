
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

#include "TypeCheck.h"

using namespace Moonshot;

TypeCheck::TypeCheck()
{
}


TypeCheck::~TypeCheck()
{
}

void TypeCheck::visit(ASTExpr * node)
{
	if (!E_CHECKSTATE) // If an error was thrown earlier, just return. We can't check the tree if it's unhealthy (and it would be pointless anyways)
		return;
	if (node->left_ && node->right_) // We've got a normal node with 2 childrens
	{
		returnTypeHelper helper(node->op_);
		// get left expr result type
		node->left_->accept(this);
		auto left = rtr_type_;
		// get right expr result type
		node->right_->accept(this);
		auto right = rtr_type_;
		// strings concat check
		if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right) && (node->op_ == parse::ADD))
			node->op_ = parse::CONCAT;

		// Check if our operation means something? If yes, the result will be placed in rtr_type.
		rtr_type_ = helper.getExprResultType(left, right);
		return;
	}
	else if(node->left_)// We only have a left node
	{
		if ((node->totype_ != parse::types::NOTYPE)) // this is a cast node, so the return type is the one of the cast node.
		{
			// Later, it might be useful to make a system like returnTypeHelper to check the validity of casts
			// But for now, almost every cast is possible, and when a cast isn't possible (string->numeric type) a warning is emitted and a default value is returned with a warning, but that is done @ runtime.
			rtr_type_ = parseTypes_toFVal(node->totype_);
			return;
		}
		else if (parse::isUnary(node->op_))
		{
			// We have a unary operation
			// Get left's return type.
			node->left_->accept(this);
			auto lefttype = rtr_type_;
			// Throw an error if it's a string. Why ? Because we can't apply the unary operators LOGICNOT or NEGATE on a string.
			if(std::holds_alternative<std::string>(lefttype))
			{
				std::stringstream ss;
				ss << "Can't perform unary operation " << getFromDict(parse::kOptype_dict, node->op_) << " on a string.";
				E_ERROR(ss.str())
			}
			if (node->op_ == parse::LOGICNOT)
			{
				rtr_type_ = FVal(false); // Return type is a boolean
				return;
			}
		}
	}
	else
	{
		// Okay, this is far-fetched, but can be possible if our parser is broken. It's better to check this here :
		// getting in this branch means that we only have a right_ node.
		E_CRITICAL("Node was in an invalid state.")
	}
	return;
}

void TypeCheck::visit(ASTValue * node)
{
	rtr_type_ = node->val_;		// Just put the value in rtr->type.
}

FVal TypeCheck::getReturnTypeOfExpr() const
{
	return rtr_type_;
}

TypeCheck::returnTypeHelper::returnTypeHelper(const parse::optype & op) : op_(op)
{

}

FVal TypeCheck::returnTypeHelper::getExprResultType(const FVal& f1, const FVal& f2)
{
	//std::cout << "Checking :" << dumpFVal(f1) << " AND " << dumpFVal(f2) << std::endl;
	std::pair<bool, FVal> result;
	// Double dispatch w/ std::visit
	std::visit([&](const auto& a, const auto& b) {
		result = getReturnType(a, b);
	}, f1, f2);
	// if success, return result.second
	if (result.first) 
		return result.second;
	else if(E_CHECKSTATE) // Try one more time if there was no error earlier, but swap the FVals
	{
		std::visit([&](const auto& a, const auto& b) {
			result = getReturnType(a, b);
		}, f2, f1); // Notice it's f2/f1 not f1/f2
		// If success, return result.second
		if (result.first)
			return result.second;
		else
			E_ERROR("Impossible operation found:");
	}
	// If error
	if (!E_CHECKSTATE)
	{
		// make an error message :
		std::stringstream ss;
		ss << "Impossible operation : " << getFromDict(parse::kOptype_dict, op_);
		ss << " between " << std::endl;
		ss << dumpFVal(f1) << std::endl << dumpFVal(f2);
		E_ERROR(ss.str());
	}
	return FVal();
}

template<typename T1, typename T2, bool isT1Num, bool isT2Num>
std::pair<bool, FVal> TypeCheck::returnTypeHelper::getReturnType(const T1 & v1, const T2 & v2)
{
	// use of if constexpr, because when functions are generated by the template, the conditions can be "decided" directly.
	if constexpr (std::is_same<T1, T2>()) // if it's the same type
	{
		// Note : Sometimes you'll see !isT1Num or isT1Num in this block of code to check if we face a string, why ?
		// Because both types are the same, so if T1 is a string, T2 is too -> we can just check if T1 is a numeric type. 
		//If it's not, it's a string, and so is T2.
		if (parse::isCondition(op_)) // Is it a condition?
		{
			if (((op_ == parse::AND) || (op_ == parse::OR)) && !isT1Num) // If we have a comp-join-op and strings, it's an error 
				E_ERROR("Operation AND (&&) or OR(||) require types convertible to boolean on each side.");
			return std::pair<bool, FVal>(true, FVal(false));	// If it's a condition, the return type will be a boolean.
		}
		else if (!isT1Num && (op_ != parse::CONCAT)) // Strings can only be concatenated 
		{
			E_ERROR("Can't perform operations other than addition (concatenation) on strings");
			return { false, FVal() };
		}
		return { true, FVal(v1) };		//the type is kept if we make a legal operation between 2 values of the same type. so we return a variant holding a sample value (v1) of the type.
	}
	else if (!isT1Num || !isT2Num) // It's 2 different types, is one of them a string ? 
		E_ERROR("Can't perform an operation on a string and a numeric type.") 		// We already know the type is different (see the first if) so we can logically assume that we have a string with a numeric type. Error!
	
	// Normal failure. We'll probably find a result when swapping T1 and T2 in getExprResultType
	return { false ,FVal() }; 
}

// Using macros for quick specialization ! Yay !
IMPL_GETRETURNTYPE(bool	,int	,true	,t_int_		)

IMPL_GETRETURNTYPE(bool	,float	,true	,t_float_	)

IMPL_GETRETURNTYPE(bool	,char	,true	,t_char_	)

IMPL_GETRETURNTYPE(int	,float	,true	,t_float_	)

IMPL_GETRETURNTYPE(int	,char	,true	,t_int_		)

IMPL_GETRETURNTYPE(char	,float	,true	,t_float_	)

