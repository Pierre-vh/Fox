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
#include "FValUtils.hpp"
#include "Moonshot/Common/Context/Context.hpp"
#include "Moonshot/Common/UTF8/StringManipulator.hpp"

#include <variant> 
#include <sstream>

using namespace Moonshot;
using namespace TypeUtils;

FVal CastUtilities::performImplicitCast(Context& context_, const FoxType& goal, FVal val)
{
	std::pair<bool, FVal> rtr = std::make_pair<bool, FVal>(false, FVal());
	std::visit(
		[&](const auto& a, const auto& b)
		{
			rtr = castTypeTo_implicit(context_,a, b);
		},
			FValUtils::getSampleFValForIndex(goal.getBuiltInTypeIndex()), val
		);

	if (rtr.first)
		return rtr.second;
	else
		context_.reportError("Failed typecast (TODO:Show detailed error message)");
	return FVal();
}

FVal CastUtilities::performExplicitCast(Context & context_, const FoxType& goal, FVal val)
{
	auto rtr = std::make_pair<bool, FVal>(false, FVal());
	std::visit(
		[&](const auto& a, const auto& b)
		{
			rtr = castTypeTo_explicit(context_, a, b);
		},
			FValUtils::getSampleFValForIndex(goal.getBuiltInTypeIndex()), val
		);

	if (rtr.first)
		return rtr.second;
	else
		context_.reportError("Failed typecast (TODO:Show detailed error message)");
	return FVal();
}

FVal CastUtilities::castTo(Context& context_, const FoxType& goal, const double & val)
{
	std::pair<bool, FVal> rtr;
	std::visit(
		[&](const auto& a)
		{
			rtr = castDoubleToArithType(context_,a, val);
		},
			FValUtils::getSampleFValForIndex(goal.getBuiltInTypeIndex())
		);
	if (rtr.first)
		return rtr.second;
	context_.reportError("Failed typecast from double (TODO:Show detailed error message");
	return FVal();
}
