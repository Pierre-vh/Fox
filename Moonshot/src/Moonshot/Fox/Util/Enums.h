////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Enums.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file declares some enumerations used in the parse	
// namespace : optype and direction	and some helper functions, to help manipulate them		
////------------------------------------------------------////

#pragma once

#include <map>
#include <cstddef>	// std::size_t

namespace Moonshot
{
	enum class operation
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

	inline bool isComparison(const operation & op) 
	{
		switch (op)
		{
			case operation::AND:
			case operation::OR:
			case operation::LESS_OR_EQUAL:
			case operation::GREATER_OR_EQUAL:
			case operation::LESS_THAN:
			case operation::GREATER_THAN:
			case operation::EQUAL:
			case operation::NOTEQUAL:
				return true;
			default:
				return false;
		}
	}
	inline bool isCompJoinOp(const operation & op)
	{
		return (op == operation::AND) || (op == operation::OR);
	}
	inline bool isUnary(const operation & op)
	{
		return (op == operation::LOGICNOT) || (op == operation::NEGATE); // Above 17 are unaries (for now)
	}

	inline constexpr bool isRightAssoc(const operation & op)
	{
		return (op == operation::EXP) || (op == operation::ASSIGN); // Only equal & exp op are right assoc.
	}

	enum class dir
	{
		UNKNOWNDIR,LEFT, RIGHT
	};
	const std::map<operation,std::string> kOptype_dict =
	{
		{ operation::CAST		, "CAST" },
		{ operation::PASS		, "PASS"	},
		{ operation::AND		, "AND"		},
		{ operation::CONCAT	, "CONCAT"	},
		{ operation::OR		, "OR"		},
		{ operation::ADD		, "ADD"		},
		{ operation::MINUS		, "MINUS"	},
		{ operation::MUL		, "MUL"		},
		{ operation::DIV		, "DIV"		},
		{ operation::MOD		, "MOD"		},
		{ operation::EXP		, "EXP"		},
		{ operation::LESS_OR_EQUAL		, "LESS_OR_EQUAL"	},
		{ operation::GREATER_OR_EQUAL	, "GREATER_OR_EQUAL"},
		{ operation::LESS_THAN			, "LESS_THAN"		},
		{ operation::GREATER_THAN		, "GREATER_THAN"	},
		{ operation::EQUAL		, "EQUAL"	},
		{ operation::NOTEQUAL	, "NOTEQUAL"},
		{ operation::LOGICNOT	, "LOGICNOT"	},
		{ operation::NEGATE	, "NEGATE"	},
		{ operation::ASSIGN	, "ASSIGN"	}
	}; 

	std::string getFromDict(const std::map<operation,std::string>& m,const operation& op);
}
