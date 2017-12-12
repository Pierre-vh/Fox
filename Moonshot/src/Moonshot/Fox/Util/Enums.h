
/************************************************************
Author : Pierre van Houtryve
Contact :
e-mail : pierre.vanhoutryve@gmail.com

Description : Various enums in the namespaces lex/parse + some dictionaries

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

#include <map>
#include <cstddef>	// std::size_t

namespace Moonshot
{
	namespace parse
	{
		enum optype
		{
			PASS,			// Just "pass" (return the value in L)
			CAST,			// "Cast" nodes
			// str concat
			CONCAT,
			// Maths.
			ADD,
			MINUS,
			MUL,
			DIV,
			MOD,
			EXP,

			// Comparison "joining" operators (&& ||)
			AND,
			OR,
			// Comparison
			LESS_OR_EQUAL,
			GREATER_OR_EQUAL,
			LESS_THAN,
			GREATER_THAN,
			EQUAL,
			NOTEQUAL,

			// Unary optypes
			LOGICNOT,		// ! 
			NEGATE		// -
		};
		
		// Template version of the other functions

		inline bool isCondition(const optype & op) 
		{
			return (op >= 9) && (op <= 15); // Condition are between 8 and 15 in the enum.
		}

		inline bool isUnary(const optype & op)
		{
			return (op >= 17); // Above 17 are unaries (for now)
		}

		inline bool isArithOp(const optype & op)
		{
			return ((op >= 3 && op <= 8) || (op == 18));
		}

		inline constexpr bool isRightAssoc(const optype & op)
		{
			return (op == EXP);
		}

		enum direction
		{
			LEFT, RIGHT
		};
		const std::map<optype,std::string> kOptype_dict =
		{
			{ CAST		, "CAST" },
			{ PASS		, "PASS"	},
			{ AND		, "AND"		},
			{ CONCAT	, "CONCAT"	},
			{ OR		, "OR"		},
			{ ADD		, "ADD"		},
			{ MINUS		, "MINUS"	},
			{ MUL		, "MUL"		},
			{ DIV		, "DIV"		},
			{ MOD		, "MOD"		},
			{ EXP		, "EXP"		},
			{ LESS_OR_EQUAL		, "LESS_OR_EQUAL"	},
			{ GREATER_OR_EQUAL	, "GREATER_OR_EQUAL"},
			{ LESS_THAN			, "LESS_THAN"		},
			{ GREATER_THAN		, "GREATER_THAN"	},
			{ EQUAL		, "EQUAL"	},
			{ NOTEQUAL	, "NOTEQUAL"},
			{ LOGICNOT	, "LOGICNOT"	},
			{ NEGATE	, "NEGATE"	}
		}; 
	}
	std::string getFromDict(const std::map<parse::optype,std::string>& m,const parse::optype& op);
}
