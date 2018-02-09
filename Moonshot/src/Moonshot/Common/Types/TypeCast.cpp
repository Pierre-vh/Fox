#include "TypeCast.hpp"

#include <variant> // std::visit
#include <sstream>
#include "Types.hpp"
#include "TypesUtils.hpp"
#include "FVTypeTraits.hpp"
#include "../Context/Context.hpp"
#include "../UTF8/StringManipulator.hpp"

using namespace Moonshot;
using namespace fv_util;

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
