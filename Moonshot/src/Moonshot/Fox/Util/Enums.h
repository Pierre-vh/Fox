
/************************************************************
Author : Pierre van Houtryve
Contact :
e-mail : pierre.vanhoutryve@gmail.com

Description : Various enums in the namespaces lex/parse

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
namespace Moonshot
{
	namespace parse
	{
		enum types
		{
			NOCAST,
			TYPE_VOID,
			TYPE_INT,
			TYPE_BOOL,
			TYPE_FLOAT,
			TYPE_STR,
			TYPE_CHAR
		};

		enum optype
		{
			DEFAULT,
			PASS,			// Just "pass" (return the value in L)

			// Comparison "joining" operators (&& ||)
			AND,
			OR,

			// Maths.
			ADD,
			MINUS,
			MUL,
			DIV,
			MOD,
			EXP,

			// Comparison
			LESS_OR_EQUAL,
			GREATER_OR_EQUAL,
			LESS_THAN,
			GREATER_THAN,
			EQUAL,
			NOTEQUAL,

			// Unary optypes
			INVERT,		// ! 
			NEGATE		// -
		};

		enum direction
		{
			LEFT, RIGHT
		};
	}
}
