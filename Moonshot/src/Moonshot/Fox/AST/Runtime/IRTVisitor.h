
/************************************************************
Author : Pierre van Houtryve
Contact :
e-mail : pierre.vanhoutryve@gmail.com

Description : Runtime visitor abstract class.
Used as a short-term solution before implementing a vm. 
Will be removed once the badger implementation is complete!

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

#pragma once
//utils
#include "../../../Common/Utils/Utils.h"
#include "../../../Common/Errors/Errors.h"
#include "../../../Common/FValue/FValue.h"

#define IRTVIS_DEFAULTBODY {return FVal();}

namespace Moonshot
{
	struct ASTExpr;
	struct ASTValue;
	struct ASTVarDeclStmt;
	class IRTVisitor
	{
		public :
			virtual ~IRTVisitor() = 0;

			inline virtual FVal visit(ASTExpr *node)			IRTVIS_DEFAULTBODY
			inline virtual FVal visit(ASTValue *node)			IRTVIS_DEFAULTBODY

			inline virtual FVal visit(ASTVarDeclStmt *node)		IRTVIS_DEFAULTBODY
	};
}