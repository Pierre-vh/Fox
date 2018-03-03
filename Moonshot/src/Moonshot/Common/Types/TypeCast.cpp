////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : TypeCast.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "TypeCast.hpp"


#include "Types.hpp"
#include "TypesUtils.hpp"
#include "FVTypeTraits.hpp"
#include "FoxValueUtils.hpp"
#include "Moonshot/Common/Context/Context.hpp"
#include "Moonshot/Common/UTF8/StringManipulator.hpp"

#include <variant> 
#include <sstream>

using namespace Moonshot;
using namespace TypeUtils;

FoxValue CastUtilities::performImplicitCast(Context& context_, const FoxType& goal, FoxValue val)
{
	std::pair<bool, FoxValue> rtr = std::make_pair<bool, FoxValue>(false, FoxValue());
	std::visit(
		[&](const auto& a, const auto& b)
		{
			using Ty = std::decay_t<decltype(a)>;
			std::pair<bool,Ty> result = castTypeTo_implicit<Ty>(context_, b);
			rtr.first = result.first;
			rtr.second = FoxValue(result.second);
		},
			FValUtils::getSampleFValForIndex(goal.getTypeIndex()), val
		);

	if (rtr.first)
		return rtr.second;
	else
		context_.reportError("Failed typecast (TODO:Show detailed error message)");
	return FoxValue();
}

FoxValue CastUtilities::performExplicitCast(Context & context_, const FoxType& goal, FoxValue val)
{
	auto rtr = std::make_pair<bool, FoxValue>(false, FoxValue());
	std::visit(
		[&](const auto& a, const auto& b)
		{
			using Ty = std::decay_t<decltype(a)>;
			std::pair<bool, Ty> result = castTypeTo_explicit<Ty>(context_, b);
			rtr.first = result.first;
			rtr.second = FoxValue(result.second);
		},
			FValUtils::getSampleFValForIndex(goal.getTypeIndex()), val
		);

	if (rtr.first)
		return rtr.second;
	else
		context_.reportError("Failed typecast (TODO:Show detailed error message)");
	return FoxValue();
}

FoxValue CastUtilities::castTo(Context& context_, const FoxType& goal, const double & val)
{
	std::pair<bool, FoxValue> rtr;
	std::visit(
		[&](const auto& a)
		{
			rtr = castDoubleToArithType(context_,a, val);
		},
			FValUtils::getSampleFValForIndex(goal.getTypeIndex())
		);
	if (rtr.first)
		return rtr.second;
	context_.reportError("Failed typecast from double (TODO:Show detailed error message");
	return FoxValue();
}
