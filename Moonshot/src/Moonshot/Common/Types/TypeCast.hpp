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

namespace Moonshot {
	class Context;
	namespace CastUtilities
	{
		// todo : Add explicit casts, with string <-> arith support.
		FVal performImplicitCast(Context& context_, const FoxType& goal, FVal val);
		FVal performExplicitCast(Context& context_, const FoxType& goal, FVal val);
		FVal castTo(Context& context_, const FoxType& goal, const double &val);

		template<typename GOAL, typename VAL, bool isGOALstr = std::is_same<GOAL, std::string>::value, bool isVALstr = std::is_same<VAL, std::string>::value>
		inline std::pair<bool, FVal> castTypeTo_implicit(Context& context_, const GOAL& type, VAL v)
		{
			if constexpr (!Traits::FValTraits<GOAL>::is_basic || !Traits::FValTraits<VAL>::is_basic)
				throw std::logic_error("Can't cast a basic type to a nonbasic type and vice versa.");
			else if constexpr((std::is_same<GOAL, VAL>::value)) // Direct conversion (between the same types)
				return { true , FVal(v) };
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
				return { false, FVal() };
			}
			else if constexpr(Traits::FValTraits<VAL>::is_arithmetic && Traits::FValTraits<GOAL>::is_arithmetic) // Arith -> Arithm = conversion works!
			{
				if constexpr (std::is_same<IntType, GOAL>::value)
					return { true,FVal((IntType)v) };
				else if constexpr (std::is_same<float, GOAL>::value)
					return { true,FVal((float)v) };
				else if constexpr (std::is_same<bool, GOAL>::value)
					return { true,FVal((bool)v) };
				else
					throw std::logic_error("Failed cast");
			}
			return { false,FVal() };
		}

		template<typename GOAL, typename VAL, bool isGOALstr = std::is_same<GOAL, std::string>::value, bool isVALstr = std::is_same<VAL, std::string>::value>
		inline std::pair<bool, FVal> castTypeTo_explicit(Context& context_, const GOAL& type, VAL v)
		{
			if constexpr (!Traits::FValTraits<GOAL>::is_basic || !Traits::FValTraits<VAL>::is_basic)
				throw std::logic_error("Can't cast a basic type to a nonbasic type and vice versa.");
			else if constexpr((std::is_same<GOAL, VAL>::value)) // Direct conversion (between the same types)
				return { true , FVal(v) };
			else if constexpr (isGOALstr && std::is_same<CharType, VAL>::value) // char -> str
			{
				std::string rtr = "";
				UTF8::append(rtr, v);
				return { true,rtr };
			}
			else if constexpr (isGOALstr != isVALstr) // One of them is a string and the other isn't.
			{
				if constexpr (isVALstr && Traits::FValTraits<GOAL>::is_arithmetic) // str -> arith
				{
					std::istringstream stream((std::string)v);
					GOAL tmp;
					if (stream >> tmp)
						return { true,FVal(tmp) };
					else
					{
						std::stringstream warnmess;
						warnmess << "Couldn't cast String \"" << (std::string)v << "\" to " << FValUtils::indexToTypeName(Traits::FValTraits<GOAL>::index) << ". Returning a base value: " << GOAL() << "\n";
						context_.reportWarning(warnmess.str());
						return { true,FVal(GOAL()) };
					}
				}
				else if constexpr (isGOALstr && Traits::FValTraits<VAL>::is_arithmetic) // arith -> str
				{
					std::stringstream stream;
					stream << v;
					return { true,FVal(stream.str()) };
				}
				else 
					throw std::logic_error("Failed cast");
			}
			else if constexpr(Traits::FValTraits<VAL>::is_arithmetic && Traits::FValTraits<GOAL>::is_arithmetic) // Arith -> Arithm = conversion works!
			{
				if constexpr (std::is_same<IntType, GOAL>::value)
					return { true,FVal((IntType)v) };
				else if constexpr (std::is_same<float, GOAL>::value)
					return { true,FVal((float)v) };
				else if constexpr (std::is_same<bool, GOAL>::value)
					return { true,FVal((bool)v) };
				else
					throw std::logic_error("Failed cast");
			}
			return { false,FVal() };
		}

		template<typename GOAL>
		inline std::pair<bool, FVal> castDoubleToArithType(Context& context_, const GOAL&, double v)
		{
			if constexpr (Traits::FValTraits<GOAL>::is_arithmetic) // Can only attempt to convert basic types.
				return { true, FVal((GOAL)v) };
			else
				throw std::logic_error("An invalid type was passed as Cast goal.");
		}
	}
}