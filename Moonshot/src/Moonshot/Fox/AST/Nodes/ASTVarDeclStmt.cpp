
/************************************************************
Author : Pierre van Houtryve
Contact :
e-mail : pierre.vanhoutryve@gmail.com

Description : See header.

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

#include "ASTVarDeclStmt.h"

using namespace Moonshot;

ASTVarDeclStmt::ASTVarDeclStmt(const var::varattr & attr, std::unique_ptr<ASTExpr>& iExpr)
{
	if (attr)
	{
		vattr_ = attr;
		if (iExpr)
			initExpr_ = std::move(iExpr);
	}
	else
		E_CRITICAL("Supplied an empty var::varattr object to the constructor.")
}

ASTVarDeclStmt::~ASTVarDeclStmt()
{

}

void ASTVarDeclStmt::accept(IVisitor * vis)
{
	VISIT_THIS
}

FVal ASTVarDeclStmt::accept(IRTVisitor * vis)
{
	VISIT_THIS
	return FVal(); // Doesn't return a value, just return something empty.
}

// varattr

var::varattr::varattr()
{
}

var::varattr::varattr(const std::string & nm, const std::size_t & ty, const bool & isK) : name(nm), type(ty), isConst(isK)
{
	wasInit_ = true;
}

var::varattr::operator bool() const
{
	return (wasInit_ && (type != fval_void) && (type != invalid_index));
}
