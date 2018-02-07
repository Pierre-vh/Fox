#pragma once

#include "Types.h"
#include "FVTypeTraits.h"
#include <map>
namespace Moonshot::fv_util
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
																	// string <-> string = ok
																	// else : error.

																	// This function returns true if the type of basetype can be cast to the type of goal.
	bool canCastTo(const std::size_t &goal, const std::size_t &basetype);

	// returns the type of the biggest of the 2 arguments.
	std::size_t getBiggest(const std::size_t &lhs, const std::size_t &rhs);

	const std::map<std::size_t, std::string> kType_dict =
	{
		{ indexes::fval_null			, "NULL" },
		{ indexes::fval_int				, "INT" },
		{ indexes::fval_float			, "FLOAT" },
		{ indexes::fval_char			, "CHAR" },
		{ indexes::fval_bool			, "BOOL" },
		{ indexes::fval_str				, "STRING" },
		{ indexes::fval_varRef			, "VAR_ATTR (ref)" },
		{ indexes::invalid_index		, "!INVALID!" }
	};
}