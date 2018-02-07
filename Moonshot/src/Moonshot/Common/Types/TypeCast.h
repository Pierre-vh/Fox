#pragma once

#include "Types.h"
#include "../Context/Context.h"

namespace Moonshot
{
	FVal castTo(Context& context_,const std::size_t& goal, FVal val);
	FVal castTo(Context& context_,const std::size_t& goal, const double &val);

	template<typename GOAL, typename VAL, bool isGOALstr = std::is_same<GOAL, std::string>::value, bool isVALstr = std::is_same<VAL, std::string>::value>
	std::pair<bool, FVal> castTypeTo(Context& context_,const GOAL& type, VAL v);

	template<typename GOAL>
	std::pair<bool, FVal> castTypeTo(Context& context_,const GOAL& type, double v);
}
