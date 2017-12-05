
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
