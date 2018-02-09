////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Enums.hpp											
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
	enum class binaryOperation
	{
		PASS,			// Just "pass" (return the value in L)
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

		// Assignement
		ASSIGN
	};
	enum class unaryOperation
	{
		DEFAULT,
		LOGICNOT,		// ! 
		NEGATIVE,			// -
		POSITIVE
	};
		
	// Template version of the other functions

	inline bool isComparison(const binaryOperation & op) 
	{
		switch (op)
		{
			case binaryOperation::AND:
			case binaryOperation::OR:
			case binaryOperation::LESS_OR_EQUAL:
			case binaryOperation::GREATER_OR_EQUAL:
			case binaryOperation::LESS_THAN:
			case binaryOperation::GREATER_THAN:
			case binaryOperation::EQUAL:
			case binaryOperation::NOTEQUAL:
				return true;
			default:
				return false;
		}
	}
	inline bool isCompJoinOp(const binaryOperation & op)
	{
		return (op == binaryOperation::AND) || (op == binaryOperation::OR);
	}

	inline constexpr bool isRightAssoc(const binaryOperation & op)
	{
		return (op == binaryOperation::EXP) || (op == binaryOperation::ASSIGN); // Only equal & exp op are right assoc.
	}

	enum class dir
	{
		UNKNOWNDIR,LEFT, RIGHT
	};
	const std::map<binaryOperation,std::string> kBinop_dict =
	{
		{ binaryOperation::PASS		, "PASS"	},
		{ binaryOperation::AND		, "AND"		},
		{ binaryOperation::CONCAT	, "CONCAT"	},
		{ binaryOperation::OR		, "OR"		},
		{ binaryOperation::ADD		, "ADD"		},
		{ binaryOperation::MINUS	, "MINUS"	},
		{ binaryOperation::MUL		, "MUL"		},
		{ binaryOperation::DIV		, "DIV"		},
		{ binaryOperation::MOD		, "MOD"		},
		{ binaryOperation::EXP		, "EXP"		},
		{ binaryOperation::LESS_OR_EQUAL	,"LESS_OR_EQUAL"	},
		{ binaryOperation::GREATER_OR_EQUAL	, "GREATER_OR_EQUAL"},
		{ binaryOperation::LESS_THAN		, "LESS_THAN"		},
		{ binaryOperation::GREATER_THAN		, "GREATER_THAN"	},
		{ binaryOperation::EQUAL			, "EQUAL"	},
		{ binaryOperation::NOTEQUAL			, "NOTEQUAL"},
		{ binaryOperation::ASSIGN			, "ASSIGN"	}
	}; 
	const std::map<unaryOperation,std::string> kUop_dict =
	{
		{ unaryOperation::DEFAULT , "DEFAULT(ERROR)"},
		{ unaryOperation::LOGICNOT , "LOGICNOT" },
		{ unaryOperation::NEGATIVE, "NEGATIVE"},
		{ unaryOperation::POSITIVE, "POSITIVE"}
	};

	template<typename T>
	inline std::string getFromDict(const std::map<T, std::string>& m, const T& op)
	{
		auto i = m.find(op);
		if (i != m.end())
			return i->second;
		return "";
	}
}
