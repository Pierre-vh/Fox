
////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Utils.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Defines some general utility variable/functions										
////------------------------------------------------------////

#pragma once

#include <memory>
#include <string>

namespace Moonshot
{
	namespace util
	{
		// Path to root
		static constexpr char moonshotSrcPath[] = ".\\..\\Moonshot\\";
		// convert enum to integer
		template<typename Enumeration>
		inline auto enumAsInt(Enumeration const value) -> typename std::underlying_type<Enumeration>::type
		{
			return static_cast<typename std::underlying_type<Enumeration>::type>(value);
		}
	}
}
