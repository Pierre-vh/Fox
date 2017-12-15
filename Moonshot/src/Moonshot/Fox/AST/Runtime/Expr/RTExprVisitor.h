
/************************************************************
Author : Pierre van Houtryve
Contact :
e-mail : pierre.vanhoutryve@gmail.com

Description : Runtime expression evaluator

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
#include "../../../../Common/Utils/Utils.h"
#include "../../../../Common/Errors/Errors.h"
#include "../../../../Common/FValue/FValue.h"

#include "../../Nodes/ASTExpr.h"

#include "../IRTVisitor.h"
#include <math.h>       /* exp */
#include <tuple>
#include <climits>
#include <cmath>

// Necessary?
//#include <variant> // std::bad_variant_access

namespace Moonshot
{
	class RTExprVisitor : public IRTVisitor
	{
		public:
			RTExprVisitor();
			~RTExprVisitor();

			// Inherited via IRTVisitor
			virtual FVal visit(ASTExpr * node) override;
			virtual FVal visit(ASTValue * node) override;
		private:
			double fvalToDouble(const FVal &fval);
			bool compareVal(const parse::optype &op, const FVal &l, const FVal &r);
			bool compareStr(const parse::optype &op, const std::string &lhs, const std::string &rhs);
			double performOp(const parse::optype& op, const double &l, const double &r);
		
			bool fitsInValue(const std::size_t& typ, const double &d); // Determines if we should convert the result to a float when making an operation to avoid loss of information
			class castHelper
			{
				public:
					FVal castTo(const std::size_t& goal, const FVal& val);
					FVal castTo(const std::size_t& goal, const double &val);
				private:
					template<typename GOAL,typename VAL, bool b1 = std::is_same<GOAL,std::string>::value,bool b2 = std::is_same<VAL,std::string>::value>
					std::pair<bool, FVal> castTypeTo(const GOAL& type,VAL v); 

					template<typename GOAL>
					std::pair<bool, FVal> castTypeTo(const GOAL& type,double v);
					
			};
	};

}

