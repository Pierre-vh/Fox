#include "TypeCast.h"

#include <variant> // std::visit
#include <sstream>
#include "Types.h"
#include "TypesUtils.h"
#include "FVTypeTraits.h"
#include "../Context/Context.h"
#include "../UTF8/StringManipulator.h"

using namespace Moonshot;
using namespace fv_util;

template<typename GOAL, typename VAL, bool isGOALstr, bool isVALstr>
std::pair<bool, FVal> Moonshot::castTypeTo(Context& context_, const GOAL& type, VAL v)
{
	if constexpr (!TypeTrait_FVal<GOAL>::is_basic || !TypeTrait_FVal<VAL>::is_basic)
		throw std::logic_error("Can't cast a basic type to a nonbasic type and vice versa.");
	else if constexpr((std::is_same<GOAL, VAL>::value)) // Direct conversion
		return { true , FVal(v) };
	else if constexpr (isGOALstr && !isVALstr) // (type) -> str
	{
		if constexpr(TypeTrait_FVal<VAL>::is_arithmetic) // arith -> str
		{
			// Casting an arithmetic type to a string:
			std::stringstream stream;
			stream << v;
			return { true, FVal(stream.str()) };
		}
		else if constexpr(std::is_same<CharType,VAL>::value) // char -> str
		{
			std::string rtr = "";
			UTF8::append(rtr, v);
			return { true,rtr };
		}
	}
	else if constexpr (isGOALstr != isVALstr) // One of them is a string and the other isn't.
	{
		std::stringstream output;
		output << "Can't convert a string to an arithmetic type and vice versa.\n";
		context_.reportError(output.str());
		return { false, FVal() };
	}
	else if constexpr(TypeTrait_FVal<VAL>::is_arithmetic && TypeTrait_FVal<GOAL>::is_arithmetic) // Arith -> Arithm = conversion works!
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
std::pair<bool, FVal> Moonshot::castTypeTo(Context& context_,const GOAL & type, double v)
{
	if constexpr(std::is_same<GOAL, std::string>::value)
	{
		throw std::logic_error("Failed cast - Attempted to cast to string.");
		return { true,FVal() };
	}
	else if constexpr (TypeTrait_FVal<GOAL>::is_basic) // Can only attempt to convert basic types.
		return { true, FVal((GOAL)v) };
	else
		throw std::logic_error("castTypeTo defaulted. Unimplemented type?");
	return { false,FVal() };
}

FVal Moonshot::castTo(Context& context_, const std::size_t& goal, FVal val)
{
	std::pair<bool, FVal> rtr = std::make_pair<bool, FVal>(false, FVal());
	std::visit(
		[&](const auto& a, const auto& b)
	{
		rtr = castTypeTo(context_,a, b);
	},
		getSampleFValForIndex(goal), val
		);

	if (rtr.first)
		return rtr.second;
	else
		context_.reportError("Failed typecast (TODO:Show detailed error message)");
	return FVal();
}

FVal Moonshot::castTo(Context& context_, const std::size_t& goal, const double & val)
{
	std::pair<bool, FVal> rtr;
	std::visit(
		[&](const auto& a)
	{
		rtr = castTypeTo(context_,a, val);
	},
		getSampleFValForIndex(goal)
		);
	if (rtr.first)
		return rtr.second;
	context_.reportError("Failed typecast from double (TODO:Show detailed error message");
	return FVal();
}
