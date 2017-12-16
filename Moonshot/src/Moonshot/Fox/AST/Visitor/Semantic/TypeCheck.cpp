#include "TypeCheck.h"

using namespace Moonshot;
using namespace fv_util;

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
	//////////////////////////////////
	/////NODES WITH 2 CHILDREN////////
	//////////////////////////////////
	if (node->left_ && node->right_) 
	{
		// VISIT BOTH CHILDREN
		// get left expr result type
		node->left_->accept(*this);
		auto left = rtr_type_;
		// get right expr result type
		node->right_->accept(*this);
		auto right = rtr_type_;
		// CHECK IF THIS IS A CONCAT OP,CONVERT IT 
		if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right) && (node->op_ == parse::ADD))
			node->op_ = parse::CONCAT;
		// CREATE HELPER
		returnTypeHelper helper(node->op_);
		// CHECK VALIDITY OF EXPRESSION
		rtr_type_ = helper.getExprResultType(left, right);

		// SPECIAL CASE: IS IT A DIVISION? IF SO,RETURN FLOAT.
		if ((node->op_ == parse::optype::DIV) && E_CHECKSTATE) // Operation is possible ? Check for division, because divisions returns float. always.
			rtr_type_ = FVal((float)0); // 
	}
	/////////////////////////////////////////
	/////NODES WITH ONLY A LEFT CHILD////////
	/////////////////////////////////////////
	else if(node->left_)// We only have a left node
	{
		// CAST NODES
		if (node->op_ == parse::CAST) // this is a cast node, so the return type is the one of the cast node. We still visit child nodes tho
		{
			// JUST VISIT CHILD, SET RTRTYPE TO THE CAST GOAL
			node->left_->accept(*this);
			rtr_type_ = getSampleFValForIndex(node->totype_);
		}
		// UNARY OPS
		else if (parse::isUnary(node->op_))
		{
			// We have a unary operation
			// Get left's return type. Don't change anything, as rtr_value is already set by the accept function.
			node->left_->accept(*this);
			auto lefttype = rtr_type_;
			// Throw an error if it's a string. Why ? Because we can't apply the unary operators LOGICNOT or NEGATE on a string.
			if(std::holds_alternative<std::string>(lefttype))
			{
				std::stringstream output;
				output << "[TYPECHECK] Can't perform unary operation " << getFromDict(parse::kOptype_dict, node->op_) << " on a string.";
				E_ERROR(output.str());
			}
			// SPECIAL CASES : (LOGICNOT)(NEGATE ON BOOLEANS)
			if (node->op_ == parse::LOGICNOT)
				rtr_type_ = FVal(false); // Return type is a boolean
			else if ((node->op_ == parse::NEGATE) && (std::holds_alternative<bool>(rtr_type_))) // If the subtree returns a boolean and we apply the negate operation, it'll return a int.
				rtr_type_ = FVal((int)0);


		}
		else
			E_CRITICAL("[TYPECHECK] A Node only had a left_ child, and wasn't a unary op.");
	}
	//////////////////////////////////
	/////ERROR CASES//////////////////
	//////////////////////////////////
	else
	{
		// Okay, this is far-fetched, but can be possible if our parser is broken. It's better to check this here :
		// getting in this branch means that we only have a right_ node.
		E_CRITICAL("[TYPECHECK] Node was in an invalid state.");
	}
	node->totype_ = rtr_type_.index();
	if (node->totype_ == invalid_index)
		E_CRITICAL("[TYPECHECK] Type was invalid.");
}

void TypeCheck::visit(ASTValue * node)
{
	rtr_type_ = node->val_;		// Just put the value in rtr->type.
}

FVal TypeCheck::getReturnTypeOfExpr() const
{
	return rtr_type_;
}

void TypeCheck::visit(ASTVarDeclStmt * node)
{
	// check for impossible/illegal assignements;
	if (node->initExpr_) // If the node has an initExpr.
	{
		// get the init expression type.
		node->initExpr_->accept(*this);
		auto iexpr_type = rtr_type_;
		// check if it's possible.
		if (!canAssign(
			node->vattr_.type,
			iexpr_type.index()
		))
		{
			E_ERROR("Can't perform initialization of variable \"" + node->vattr_.name + "\"");
		}
	}
	// Else, sadly, we can't really check anything @ compile time.
}

TypeCheck::returnTypeHelper::returnTypeHelper(const parse::optype & op) : op_(op)
{

}

FVal TypeCheck::returnTypeHelper::getExprResultType(const FVal& f1, const FVal& f2)
{
	// first, quick, simple check : we can only verify results between 2 basic types.
	if (!isBasic(f1.index()) || !isBasic(f2.index()))
	{
		if(!E_CHECKSTATE) // Don't throw an error twice.
			E_ERROR("[TYPECHECK] Can't typecheck an expression where lhs,rhs or both sides aren't basic types (int/char/bool/string/float).");
		return FVal();
	}
	// This function will simply "visit" (using std::visit) both fval so we can call a function depending on the stored type.
	// It'll do it twice, and invert the arguments for the second time. For instance, float,int might not be a recognized case, but int,float is.

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
			E_ERROR("[TYPECHECK] Impossible operation found: Unimplemented type/operation?"); // It's 
	}
	// If error
	if (!E_CHECKSTATE)
	{
		// make an error message :
		std::stringstream output;
		output << "[TYPECHECK] Impossible operation : " << getFromDict(parse::kOptype_dict, op_);
		output << " between " << std::endl;
		output << dumpFVal(f1) << std::endl << dumpFVal(f2);
		E_ERROR(output.str());
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
		if (parse::isComparison(op_)) // Is it a condition?
		{
			if (((op_ == parse::AND) || (op_ == parse::OR)) && !isT1Num)											// If we have a comp-join-op and strings, it's an error 
			{
				E_ERROR("Operations AND (&&) and OR (||) require types convertible to boolean on each side.");
				return { false, FVal() };
			}
			return { true,FVal(false) };	//f it's a condition, the return type will be a boolean.
		}
		else if (!isT1Num && (op_ != parse::CONCAT)) // Strings can only be concatenated Or Compared.
		{
			E_ERROR("[TYPECHECK] Can't perform operations other than addition (concatenation) on strings");
			return	{ false, FVal() };
		}
		else if (std::is_same<bool, T1>::value && parse::isArithOp(op_))
		{
			// We have 2 booleans, the result of an arithmetic operation between them is a int!
			std::stringstream output;
			output << "[TYPECHECK] The result of an artihmetic operation between 2 boolean is an integer ! "
				<< std::endl
				<< "Operation concerned: [" << v1 << " " << getFromDict(parse::kOptype_dict, op_) << " " << v2 << "]" << std::endl;
			E_WARNING(output.str());
				return	{ true, FVal() };
		}

		return { true, FVal(v1) };		//the type is kept if we make a legal operation between 2 values of the same type. so we return a variant holding a sample value (v1) of the type.
	}
	else if constexpr (!isT1Num || !isT2Num) // It's 2 different types, is one of them a string ? 
		E_ERROR("[TYPECHECK] Can't perform an operation on a string and a numeric type."); 		// We already know the type is different (see the first if) so we can logically assume that we have a string with a numeric type. Error!
	else if (parse::isComparison(op_))
		return { true, FVal(false) };
	// Normal failure. We'll probably find a result when swapping T1 and T2 in getExprResultType.
	return { false ,FVal() }; 
}

// Using macros for quick specialization ! Yay !
IMPL_GETRETURNTYPE(bool	,int	,true	,t_int_		)

IMPL_GETRETURNTYPE(bool	,float	,true	,t_float_	)

IMPL_GETRETURNTYPE(bool	,char	,true	,t_char_	)

IMPL_GETRETURNTYPE(int	,float	,true	,t_float_	)

IMPL_GETRETURNTYPE(int	,char	,true	,t_int_		)

IMPL_GETRETURNTYPE(char	,float	,true	,t_float_	)

