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
	// tip : split everything into functions, and make everything as generic as possible to avoid lot of repeating code
	// tip : use std::swap to check "int/float" & "float/int" for instance. (call the comparison function a second time with values swapped when needed!"
	// tip : maybe eliminate error cases (when strings are involved) first to clear error possibilities?
	// Negate,Logicnot operators and every arithmetic operator except + won't work on strings 
	// Comparisons will only work on 2 strings
	// + on 2 strings should be converted to CONCAT (new operation) -> Only modification of the node allowed !
	// Pretty much everything's allowed for arithmetic operators/comparisons between 2 numerical value
	// Refer to https://prnt.sc/hh53fz for operations return types. 
	// Don't forget that ! will always return a boolean
	// Comparisons, when they're possible, will always return booleans.
}

void TypeCheck::visit(ASTValue * node)
{
	rtr_type_ = getTypeFromFVal(node->val_);
}
