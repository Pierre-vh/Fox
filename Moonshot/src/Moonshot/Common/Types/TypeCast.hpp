////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : TypeCast.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Defines various type casting utilities
////------------------------------------------------------////

#pragma once

#include "Types.hpp"
#include "FVTypeTraits.hpp"
#include <sstream>
#include <string>

/*
	TO DO
	Rework this class. It works really well right now, but I don't really like std::visit, and in the future
	if it add more complex types	(arrays,tuples,...) (or oop) it's going to break real quick. I'll need to make a complete casting utility that manages every possible case and outputs clear, detailed error
	messages.

	How to rework it:
	Ditch templates, use a switch with Fox-Type and specialized cast function for every case:
		ArithTostring
		StringToArith
		ArithToArith
		...you name it.

	Really, std::visit is useless there and adds unwanted complexity.

	Note, both of the castTypeTo method will throw if you request a bad cast, because the TypeChecker is supposed to check the validity of every cast before
	this function is called.
*/
namespace Moonshot {
	class Context;
	namespace CastUtilities
	{
		// todo : Add explicit casts, with string <-> arith support.
		FoxValue performImplicitCast(Context& context_, const FoxType& goal, FoxValue val);
		FoxValue performExplicitCast(Context& context_, const FoxType& goal, FoxValue val);
		FoxValue castTo(Context& context_, const FoxType& goal, const double &val);

		template<typename GOAL, typename VAL, bool isGOALstr = std::is_same<GOAL, std::string>::value, bool isVALstr = std::is_same<VAL, std::string>::value>
		inline std::pair<bool,GOAL> castTypeTo_implicit(Context& context_, VAL v)
		{
			if constexpr (!Traits::FoxValueTraits<GOAL>::is_basic || !Traits::FoxValueTraits<VAL>::is_basic)
				throw std::logic_error("Can't cast a basic type to a nonbasic type and vice versa.");
			else if constexpr((std::is_same<GOAL, VAL>::value)) // Direct conversion (between the same types)
				return { true , v };
			else if constexpr (isGOALstr && std::is_same<CharType, VAL>::value) // char -> str
			{
				std::string rtr = "";
				UTF8::append(rtr, v);
				return { true,rtr };
			}
			else if constexpr (isGOALstr != isVALstr) // One of them is a string and the other isn't.
			{
				std::stringstream output;
				output << "Can't convert a string to an arithmetic type and vice versa.\n";
				context_.reportError(output.str());
				return { false, GOAL() };
			}
			else if constexpr(Traits::FoxValueTraits<VAL>::is_arithmetic && // Arith -> Int/float/bool = ok
				((std::is_same<IntType, GOAL>::value || std::is_same<float, GOAL>::value || std::is_same<bool, GOAL>::value)))
			{
				return { true,(GOAL)v };
			}
			throw std::logic_error("Failed cast");
			return { false,GOAL() };
		}

		template<typename GOAL, typename VAL, bool isGOALstr = std::is_same<GOAL, std::string>::value, bool isVALstr = std::is_same<VAL, std::string>::value>
		inline std::pair<bool, GOAL> castTypeTo_explicit(Context& context_,VAL v)
		{
			if constexpr (!Traits::FoxValueTraits<GOAL>::is_basic || !Traits::FoxValueTraits<VAL>::is_basic)
				throw std::logic_error("Can't cast a basic type to a nonbasic type and vice versa.");
			else if constexpr((std::is_same<GOAL, VAL>::value)) // Direct conversion (between the same types)
				return { true ,v };
			else if constexpr (isGOALstr != isVALstr) // One of them is a string and the other isn't.
			{
				if constexpr (isVALstr && Traits::FoxValueTraits<GOAL>::is_arithmetic) // str -> arith
				{
					std::istringstream stream((std::string)v);
					GOAL tmp;
					if (stream >> tmp)
						return { true,tmp };
					else
					{
						std::stringstream warnmess;
						warnmess << "Couldn't cast String \"" << (std::string)v << "\" to " << FValUtils::getTypenameForIndex(Traits::FoxValueTraits<GOAL>::index) << ". Returning a base value: " << GOAL() << "\n";
						context_.reportWarning(warnmess.str());
						return { true,GOAL() };
					}
				}
				else if constexpr (isGOALstr && std::is_same<CharType, VAL>::value) // char -> str
				{
					std::string rtr = "";
					UTF8::append(rtr, v);
					return { true,rtr };
				}
				else if constexpr (isGOALstr && Traits::FoxValueTraits<VAL>::is_arithmetic) // arith -> str
				{
					std::stringstream stream;
					stream << v;
					return { true,stream.str()};
				}
			}
			else if constexpr(Traits::FoxValueTraits<VAL>::is_arithmetic && // Arith -> Int/float/bool = ok
							((std::is_same<IntType, GOAL>::value || std::is_same<float, GOAL>::value || std::is_same<bool, GOAL>::value)))
			{
				return { true,(GOAL)v };
			}
			throw std::logic_error("Failed cast");
			return { false,GOAL() };
		}

		template<typename GOAL>
		inline std::pair<bool, FoxValue> castDoubleToArithType(Context& context_, const GOAL&, double v)
		{
			if constexpr (Traits::FoxValueTraits<GOAL>::is_arithmetic) // Can only attempt to convert basic types.
				return { true, FoxValue((GOAL)v) };
			else
				throw std::logic_error("An invalid type was passed as Cast goal.");
		}
	}
}