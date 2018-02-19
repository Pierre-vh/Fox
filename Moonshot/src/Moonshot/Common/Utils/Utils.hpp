
////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Utils.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Defines some general utility variable/functions										
////------------------------------------------------------////

#pragma once

#include <string>
#include <map>
namespace Moonshot
{
	namespace Util
	{
		// Path to root
		static const std::string moonshotSrcPath = ".\\..\\Moonshot\\";
		// convert enum to integer
		template<typename Enumeration>
		inline auto enumAsInt(Enumeration const value) -> typename std::underlying_type<Enumeration>::type
		{
			return static_cast<typename std::underlying_type<Enumeration>::type>(value);
		}

		template<typename T>
		inline std::string getFromDict(const std::map<T, std::string>& m, const T& op)
		{
			auto i = m.find(op);
			if (i != m.end())
				return i->second;
			return "";
		}
	}
}
