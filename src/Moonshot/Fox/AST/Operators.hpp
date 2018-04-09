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
		DEFAULT,			
		CONCAT,	// +
		// Maths.
		ADD,	// +
		MINUS,	// -
		MUL,	// *
		DIV,	// /
		MOD,	// %
		EXP,	// **
		// Logical and and or
		LOGIC_AND,	// &&
		LOGIC_OR,	// ||
		// Comparison
		LESS_OR_EQUAL,		// <=
		GREATER_OR_EQUAL,	// >=
		LESS_THAN,		// <
		GREATER_THAN,	// >
		EQUAL,			// ==
		NOTEQUAL,		// !=

		// Assignement
		ASSIGN_BASIC	// =
	};
	enum class unaryOperator
	{
		DEFAULT,
		LOGICNOT,		// ! 
		NEGATIVE,		// -
		POSITIVE		// +
	};

	namespace Dicts
	{
		const std::map<binaryOperator, std::string> kBinopToStr_dict =
		{
			{ binaryOperator::DEFAULT	, "[ENUM_DEFAULT]" },
			{ binaryOperator::LOGIC_AND	, "LOGIC_AND" },
			{ binaryOperator::CONCAT	, "CONCAT" },
			{ binaryOperator::LOGIC_OR	, "LOGIC_OR" },
			{ binaryOperator::ADD		, "ADD" },
			{ binaryOperator::MINUS		, "MINUS" },
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
			{ binaryOperator::ASSIGN_BASIC		, "ASSIGN_BASIC" }
		};

		const std::map<unaryOperator, std::string> kUnaryOpToStr_dict =
		{
			{ unaryOperator::DEFAULT	, "[ENUM_DEFAULT]" },
			{ unaryOperator::LOGICNOT	, "LOGICNOT" },
			{ unaryOperator::NEGATIVE	, "NEGATIVE" },
			{ unaryOperator::POSITIVE	, "POSITIVE" }
		};
	}
}
