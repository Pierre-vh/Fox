////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : FoxValueUtils.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This class defines various utility functions for FoxValues
////------------------------------------------------------////

#pragma once
#include "Types.hpp"
#include <map>
namespace Moonshot::FValUtils
{
	bool isBasic(const FoxValue& fv);
	bool isArithmetic(const FoxValue& fv);
	bool isValue(const FoxValue& fv);

	std::string dumpFVal(const FoxValue& fv);

	// Get a FoxValue containing a sample value for an index
	FoxValue getSampleFValForIndex(const std::size_t& t);

	// Returns the name of the FoxValue's current type.
	std::string getFValTypeName(const FoxValue& t);
	std::string getTypenameForIndex(const std::size_t& ind);

	const std::map<std::size_t, std::string> kBuiltinTypes_dict =
	{
		{ TypeIndex::Void_Type			, "VOID" },
		{ TypeIndex::basic_Int			, "INT" },
		{ TypeIndex::basic_Float		, "FLOAT" },
		{ TypeIndex::basic_Char			, "CHAR" },
		{ TypeIndex::basic_Bool			, "BOOL" },
		{ TypeIndex::basic_String		, "STRING" },
		{ TypeIndex::InvalidIndex		, "!INVALID!" }
	};
}