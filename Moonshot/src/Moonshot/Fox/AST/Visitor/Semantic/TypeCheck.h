
/************************************************************
Author : Pierre van Houtryve
Contact :
e-mail : pierre.vanhoutryve@gmail.com

Description : Type checking for expressions
This class checks the return type of the whole expression (and E_ERROR when it found a impossible operation)
You can get the result using getReturnTypeOfExpr().

This class sets totype_ variable as the return type of the expression, and changes the ADD operation between 2 strings to concat.
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
//
#pragma once
#include <string> // std::string
#include <type_traits> // std::is_same
#include <sstream> // std::stringstream
#include <variant> // std::visit
#include <typeinfo> // typeid
#include "../IVisitor.h" // base class
#include "../../../../Common/FValue/FValue.h" // FVal Utilities
#include "../../Nodes/ASTExpr.h" // Include nodes

// remember : typedef std::variant<int, float, char, std::string, bool> FVal;

#define DECL_GETRETURNTYPE(x,y)	template<>\
									std::pair <bool, FVal> getReturnType(const x& v1, const y& v2);\

#define IMPL_GETRETURNTYPE(x,y,b,v) template<>\
								std::pair<bool, FVal> TypeCheck::returnTypeHelper::getReturnType(const x& v1, const y& v2)\
								{return {b,FVal(v)};} \

namespace Moonshot
{
	class TypeCheck : public IVisitor
	{
		public:
			TypeCheck();
			~TypeCheck();

			virtual void visit(ASTExpr * node) override;
			virtual void visit(ASTValue * node) override;

			FVal getReturnTypeOfExpr() const;
		private:
			FVal rtr_type_; // Last returned type from visiting node (held here, because visit doesn't return anything :( )
			struct returnTypeHelper
			{
				public:
					returnTypeHelper(const parse::optype &op);
					FVal getExprResultType(const FVal& f1, const FVal& f2);
				private:
				// There are 5 different types
				// 5^2 -> 25 Different interactions for binary operators.
				// Except we'll do something : If no overload is found, we try to find one again but this time we swap the types.
				// So if there is no specialization for float,int we test again with int,float and return an error when this one returns nothing.
				// With that we can eliminate a lot of cases, like half of them.
				// We can also handle in the default function T1 == T2 (same type) and just return T1's type.
				// We just have a few cases to implement. Just 6 !
					template <typename T1, typename T2, bool isT1Num = std::is_arithmetic<T1>::value,bool isT2Num = std::is_arithmetic<T2>::value>
					std::pair <bool,FVal> getReturnType(const T1& v1, const T2& v2); // The bool is used to indicate if the return type was successfully determinated

					DECL_GETRETURNTYPE(bool, int)
					DECL_GETRETURNTYPE(bool, float)
					DECL_GETRETURNTYPE(bool, char)

					DECL_GETRETURNTYPE(int, float)
					DECL_GETRETURNTYPE(int, char)
				
					DECL_GETRETURNTYPE(char,float)

					private:

						static constexpr int t_int_ = 0;
						static constexpr float t_float_ = 0.0f;
						static constexpr char t_char_ = ' ';
						static constexpr char t_str_[] = ""; // can't use constexpr strings, so I use a char array.
						static constexpr bool t_bool_ = false;

						parse::optype op_;
			};
	};  

}

