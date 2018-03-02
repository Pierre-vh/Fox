////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : TypesUtils.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "TypesUtils.hpp"
#include <sstream>
#include "../UTF8/StringManipulator.hpp"

using namespace Moonshot;

std::string TypeUtils::dumpFVal(const FVal & var)
{
	std::stringstream output;
	if (std::holds_alternative<NullType>(var))
		output << "Type : VOID (null)";
	else if (std::holds_alternative<var::varRef>(var))
	{
		auto vattr = std::get<var::varRef>(var);
		output << "Type : varRef, Value:" << vattr.getName();
	}
	else if (std::holds_alternative<IntType>(var))
		output << "Type : INT, Value : " << std::get<IntType>(var);
	else if (std::holds_alternative<float>(var))
		output << "Type : FLOAT, Value : " << std::get<float>(var);
	else if (std::holds_alternative<std::string>(var))
		output << "Type : STRING, Value : \"" << std::get<std::string>(var) << "\"";
	else if (std::holds_alternative<bool>(var))
	{
		const bool v = std::get<bool>(var);
		output << "Type : BOOL, Value : " << (v ? "true" : "false");
	}
	else if (std::holds_alternative<CharType>(var))
	{
		CharType x = std::get<CharType>(var);
		UTF8::StringManipulator u8sm;
		output << "Type : CHAR, Value : " << x << " = '" << u8sm.wcharToStr(x) << "'";
	}
	else
		throw std::logic_error("Illegal variant.");
	return output.str();
}
std::string TypeUtils::dumpVAttr(const var::varattr & var)
{
	std::stringstream output;
	output << "[name:\"" << var.name_ << "\" "
		<< "type: " << (var.isConst_ ? "CONST " : "");
	auto friendlyname = kType_dict.find(var.type_);
	if (friendlyname != kType_dict.end())
		output << friendlyname->second;
	else
		output << "<UNKNOWN>";
	output << "]";
	return output.str();
}
FVal TypeUtils::getSampleFValForIndex(const std::size_t & t)
{
	switch (t)
	{
	case Types::builtin_Null:
		return FVal();
	case Types::basic_Int:
		return FVal((IntType)0);
	case Types::basic_Float:
		return FVal((float)0.0f);
	case Types::basic_Char:
		return FVal((CharType)0);
	case Types::basic_String:
		return FVal(std::string(""));
	case Types::basic_Bool:
		return FVal((bool)false);
	case Types::builtin_VarRef:
		return FVal(var::varattr());
	case Types::InvalidIndex:
		throw std::logic_error("Tried to get a sample FVal with an invalid index");
		return FVal();
	default:
		throw std::logic_error("Defaulted while attempting to return a sample FVal for an index. -> Unknown index. Unimplemented type?");
		return FVal();
	}
}

std::string TypeUtils::indexToTypeName(const std::size_t & t)
{
	auto a = kType_dict.find(t);
	if (a != kType_dict.end())
		return a->second;
	return "!IMPOSSIBLE_TYPE!";
}

bool TypeUtils::canAssign(const std::size_t & lhs, const std::size_t & rhs)
{
	if ((rhs == Types::builtin_Null) || (lhs == Types::builtin_Null))
		return false; // Can't assign a void expression to a variable.
	if (!isBasic(lhs) || !isBasic(rhs))
		// If one of the types isn't basic, no assignement possible.
		return false;
	else if (lhs == rhs) // same type to same type = ok.
		return true;
	// From here, we know lhs and rhs are different.
	else if (!isArithmetic(lhs) || !isArithmetic(rhs)) // one of them is a string
		return false;  // Can't assign a string to an arithmetic type and vice versa.
					   // Else, we're good, return true.
	return true;
}
bool TypeUtils::canImplicitelyCastTo(const std::size_t & goal, const std::size_t & basetype)
{
	/*
		Implicit Conversions:
		Arith type -> Arith type
		char type -> string type
		same type -> same type
	*/
	if (isBasic(basetype))
	{
		if (isArithmetic(goal) && isArithmetic(basetype)) // arith -> arith
			return true;
		else if ((basetype == Types::basic_Char) && (goal == Types::basic_String)) // char -> str
			return true;
		return (basetype == goal); // same type -> same type
	}
	return false;
}

bool TypeUtils::canExplicitelyCastTo(const std::size_t & goal, const std::size_t & basetype)
{
	/*
		Implicit rules + strings <-> arith support (string as int to interpret it as int, int as string to convert it to string)
	*/
	if (canImplicitelyCastTo(goal, basetype))
		return true;
	else
	{
		return	((goal == Types::basic_String) && isArithmetic(basetype)) || // arith -> str
				(isArithmetic(goal) && (basetype == Types::basic_String)); // str -> arith
	}
	return false;
}

std::size_t TypeUtils::getBiggest(const std::size_t & lhs, const std::size_t & rhs)
{
	if (isArithmetic(lhs) && isArithmetic(rhs))
	{
		if ((lhs == Types::basic_Float) || (rhs == Types::basic_Float))
			return Types::basic_Float;
		else if ((lhs == Types::basic_Int) || (rhs == Types::basic_Int))
			return Types::basic_Int;
		else if ((lhs == Types::basic_Char) || (rhs == Types::basic_Char))
			return Types::basic_Char;
		else
			return Types::basic_Bool;
	}
	else
		throw std::logic_error("Can't return the biggest of two types when one of the two type isn't arithmetic.");
	return Types::InvalidIndex;
}
