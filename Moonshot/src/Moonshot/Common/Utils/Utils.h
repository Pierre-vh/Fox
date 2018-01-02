#pragma once

#include <memory>
#include <string>

namespace Moonshot
{
	namespace util
	{
		//open a file in the moonshot folder from the build folder
		std::string filepath_MoonshotProj(const std::string &s);
		// convert enum to integer
		template<typename Enumeration>
		inline auto enumAsInt(Enumeration const value) -> typename std::underlying_type<Enumeration>::type
		{
			return static_cast<typename std::underlying_type<Enumeration>::type>(value);
		}
	}
}
