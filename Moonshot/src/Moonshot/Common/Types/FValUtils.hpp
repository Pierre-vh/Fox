////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : FValUtils.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This class defines various utility functions for FVals
////------------------------------------------------------////

#pragma once
#include "Types.hpp"
#include <map>
namespace Moonshot::FValUtils
{
	bool isBasic(const FVal& fv);
	bool isArithmetic(const FVal& fv);
	bool isValue(const FVal& fv);

	std::string dumpFVal(const FVal& fv);

	// Get a FVal containing a sample value for an index
	FVal getSampleFValForIndex(const std::size_t& t);

	// Returns the name of the FVal's current type.
	std::string getFValTypeName(const FVal& t);
	std::string getTypenameForIndex(const std::size_t& ind);

	const std::map<std::size_t, std::string> kBuiltinTypes_dict =
	{
		{ TypeIndex::Null_Type			, "NULL" },
		{ TypeIndex::basic_Int			, "INT" },
		{ TypeIndex::basic_Float		, "FLOAT" },
		{ TypeIndex::basic_Char			, "CHAR" },
		{ TypeIndex::basic_Bool			, "BOOL" },
		{ TypeIndex::basic_String		, "STRING" },
		{ TypeIndex::VarRef				, "VAR_ATTR (ref)" },
		{ TypeIndex::InvalidIndex		, "!INVALID!" }
	};
}