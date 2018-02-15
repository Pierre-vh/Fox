////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Enums.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file declares the Operators enumeration with some utilities functions
////------------------------------------------------------////

#pragma once

#include <map>

namespace Moonshot
{
	enum class binaryOperator
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
	enum class unaryOperator
	{
		DEFAULT,
		LOGICNOT,		// ! 
		NEGATIVE,		// -
		POSITIVE		// +
	};

	inline bool isComparison(const binaryOperator & op)
	{
		switch (op)
		{
			case binaryOperator::AND:
			case binaryOperator::OR:
			case binaryOperator::LESS_OR_EQUAL:
			case binaryOperator::GREATER_OR_EQUAL:
			case binaryOperator::LESS_THAN:
			case binaryOperator::GREATER_THAN:
			case binaryOperator::EQUAL:
			case binaryOperator::NOTEQUAL:
				return true;
			default:
				return false;
		}
	}
	namespace Dicts
	{
		const std::map<binaryOperator, std::string> kBinopToStr_dict =
		{
			{ binaryOperator::PASS		, "PASS" },
			{ binaryOperator::AND		, "AND" },
			{ binaryOperator::CONCAT	, "CONCAT" },
			{ binaryOperator::OR		, "OR" },
			{ binaryOperator::ADD		, "ADD" },
			{ binaryOperator::MINUS		,	"MINUS" },
			{ binaryOperator::MUL		, "MUL" },
			{ binaryOperator::DIV		, "DIV" },
			{ binaryOperator::MOD		, "MOD" },
			{ binaryOperator::EXP		, "EXP" },
			{ binaryOperator::LESS_OR_EQUAL		,"LESS_OR_EQUAL" },
			{ binaryOperator::GREATER_OR_EQUAL	, "GREATER_OR_EQUAL" },
			{ binaryOperator::LESS_THAN			, "LESS_THAN" },
			{ binaryOperator::GREATER_THAN		, "GREATER_THAN" },
			{ binaryOperator::EQUAL				, "EQUAL" },
			{ binaryOperator::NOTEQUAL			, "NOTEQUAL" },
			{ binaryOperator::ASSIGN			, "ASSIGN" }
		};

		const std::map<unaryOperator, std::string> kUnaryOpToStr_dict =
		{
			{ unaryOperator::DEFAULT	, "[DEFAULT]" },
			{ unaryOperator::LOGICNOT	, "LOGICNOT" },
			{ unaryOperator::NEGATIVE	, "NEGATIVE" },
			{ unaryOperator::POSITIVE	, "POSITIVE" }
		};
	}
}
