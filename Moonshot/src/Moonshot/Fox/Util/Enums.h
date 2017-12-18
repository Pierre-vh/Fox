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
			NEGATE,		// -

			// Assignement
			ASSIGN
		};
		
		// Template version of the other functions

		inline bool isComparison(const optype & op) 
		{
			return (op >= 9) && (op <= 16); // Condition are between 8 and 15 in the enum.
		}
		inline bool isCompJoinOp(const optype & op)
		{
			return (op == AND) || (op == OR);
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
			return (op == EXP) || (op == ASSIGN); // Only equal & exp op are right assoc.
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
			{ NEGATE	, "NEGATE"	},	
			{ ASSIGN	, "ASSIGN"	}
		}; 
	}
	std::string getFromDict(const std::map<parse::optype,std::string>& m,const parse::optype& op);
}
