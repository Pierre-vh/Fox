////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : TypesUtils.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "TypesUtils.hpp"
#include "FVTypeTraits.hpp"
#include "../UTF8/StringManipulator.hpp"

#include <sstream>
using namespace Moonshot;

bool TypeUtils::canAssign(const FoxType & lhs, const FoxType & rhs)
{
	if ((rhs == TypeIndex::Void_Type) || (lhs == TypeIndex::Void_Type))
		return false;							// Can't assign a void expression to a variable.
	if (!lhs.isBasic() || !rhs.isBasic())
		return false;							// If one of the types isn't basic, no assignement possible.
	else if (lhs.isConst())						// if the lhs (the assigned value) is a constant, we do a permissive check, because we can assign something to it even if (strictly speaking) the types are different.
		return lhs.compareWith_permissive(rhs); // (of course,we assume it's a variable declaration scenario.)
	else if (lhs == rhs)						// exact same type to exact same type
		return true;							// = ok
												// From here, we know lhs and rhs are different.
	else if (!lhs.isArithmetic() || !rhs.isArithmetic()) // one of them is a string, the other isn't, nope!
		return (lhs.is(TypeIndex::basic_String) && rhs.is(TypeIndex::basic_Char));  // if lhs = string and rhs=char it's ok, all other cases aren't.
					   // Else, we're good, return true.
	return true;
}
bool TypeUtils::canImplicitelyCastTo(const FoxType & goal, const FoxType & basetype)
{
	/*
		Implicit Conversions:
		Arith type -> Arith type
		char type -> string type
		same type -> same type
	*/
	if (basetype.isBasic())
	{
		if (goal.isArithmetic() && basetype.isArithmetic()) // arith -> arith
			return true;
		else if ((basetype == TypeIndex::basic_Char) && (goal == TypeIndex::basic_String)) // char -> str
			return true;
		return (basetype == goal); // same type -> same type
	}
	return false;
}

bool TypeUtils::canExplicitelyCastTo(const FoxType & goal, const FoxType & basetype)
{
	/*
		Implicit rules + strings <-> arith support (string as int to interpret it as int, int as string to convert it to string)
	*/
	if (canImplicitelyCastTo(goal, basetype))
		return true;
	else
	{
		return	((goal == TypeIndex::basic_String) && basetype.isArithmetic()) || // arith -> str
				(goal.isArithmetic() && (basetype == TypeIndex::basic_String)); // str -> arith
	}
	return false;
}

bool TypeUtils::canConcat(const FoxType & lhs, const FoxType & rhs)
{
	if (lhs.isBasic() && rhs.isBasic())
	{
		switch (lhs.getTypeIndex())
		{
			case TypeIndex::basic_Char:
			case TypeIndex::basic_String:
				return	(rhs == TypeIndex::basic_Char) || (rhs == TypeIndex::basic_String);
			default:
				return false;
		}
	}
	return false;
}

FoxType TypeUtils::getBiggestType(const FoxType & lhs, const FoxType & rhs)
{
	if (lhs.isArithmetic() && rhs.isArithmetic())
	{
		if ((lhs == TypeIndex::basic_Float) || (rhs == TypeIndex::basic_Float))
			return TypeIndex::basic_Float;
		else if ((lhs == TypeIndex::basic_Int) || (rhs == TypeIndex::basic_Int))
			return TypeIndex::basic_Int;
		else if ((lhs == TypeIndex::basic_Char) || (rhs == TypeIndex::basic_Char))
			return TypeIndex::basic_Char;
		else
			return TypeIndex::basic_Bool;
	}
	else
		throw std::logic_error("Can't return the biggest of two types when one of the two type isn't arithmetic.");
	return TypeIndex::InvalidIndex;
}
