
/************************************************************
Author : Pierre van Houtryve
Contact :
e-mail : pierre.vanhoutryve@gmail.com

Description : Variable Declaration Statement AST Node.

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

#include "IASTStmt.h"
#include "ASTExpr.h"

#include <string> // std::string
#include <memory> // std::unique_ptr, std::make_unique

namespace Moonshot 
{
	namespace var
	{
		struct varattr // Struct holding a var's attributes
		{
			varattr();
			varattr(const std::string &nm, const std::size_t &ty, const bool &isK = false);
			operator bool() const;
			// Variable's attribute
			bool isConst = false;
			std::string name = "";
			std::size_t type = fval_void;

			private:
				bool wasInit_ = false;
		};
	}
	struct ASTVarDeclStmt : public IASTStmt
	{
		public:
			// Create a variable declaration statement by giving the constructor the variable's properties (name,is const and type) and, if there's one, an expression to initialize it.
			ASTVarDeclStmt(const var::varattr &attr,std::unique_ptr<ASTExpr>& iExpr = std::make_unique<ASTExpr>(nullptr)); 
			~ASTVarDeclStmt();

			// Inherited via IASTStmt
			virtual void accept(IVisitor * vis) override;
			virtual FVal accept(IRTVisitor * vis) override;

			var::varattr vattr_;
			std::unique_ptr<ASTExpr> initExpr_;
	};
}


