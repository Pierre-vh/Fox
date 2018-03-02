#pragma once

#include "Types.hpp"
#include "FVTypeTraits.hpp"
#include <map>
namespace Moonshot::TypeUtils
{
	// Dump functions
	std::string dumpFVal(const FVal &var);
	std::string dumpVAttr(const var::varattr &var);

	// returns a sample fval for an index.
	FVal getSampleFValForIndex(const std::size_t& t);

	// Get a user friendly name for an index.
	std::string indexToTypeName(const std::size_t& t);

	// Checks if assignement is possible.
	bool canAssign(const std::size_t &lhs, const std::size_t &rhs); // Checks if the lhs and rhs are compatible.
																	// Compatibility : 
																	// Arithmetic type <-> Arithmetic Type = ok
																	// same type <-> same type = ok
																	// else : error.

																	// This function returns true if the type of basetype can be cast to the type of goal.
	bool canImplicitelyCastTo(const std::size_t &goal, const std::size_t &basetype);
	bool canExplicitelyCastTo(const std::size_t &goal, const std::size_t &basetype);

	inline constexpr bool canConcat(const std::size_t& lhs, const std::size_t& rhs)
	{
		if (isBasic(lhs) && isBasic(rhs))
		{
			switch (lhs)
			{
				case Types::basic_Char:
				case Types::basic_String:
					return	(rhs == Types::basic_Char) || (rhs == Types::basic_String);
				default:
					return false;
			}
		}
		return false;
	}
	// returns the type of the biggest of the 2 arguments.
	std::size_t getBiggest(const std::size_t &lhs, const std::size_t &rhs);

	const std::map<std::size_t, std::string> kType_dict =
	{
		{ Types::builtin_Null			, "NULL" },
		{ Types::basic_Int				, "INT" },
		{ Types::basic_Float			, "FLOAT" },
		{ Types::basic_Char			, "CHAR" },
		{ Types::basic_Bool			, "BOOL" },
		{ Types::basic_String				, "STRING" },
		{ Types::builtin_VarRef			, "VAR_ATTR (ref)" },
		{ Types::InvalidIndex		, "!INVALID!" }
	};
}