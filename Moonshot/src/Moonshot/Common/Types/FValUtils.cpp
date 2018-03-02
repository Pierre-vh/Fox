////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : FValUtils.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "FValUtils.hpp"
#include "Moonshot/Common/Types/FVTypeTraits.hpp"
#include "Moonshot/Common/UTF8/StringManipulator.hpp"

#include <sstream>

using namespace Moonshot;

bool FValUtils::isBasic(const FVal & fv)
{
	return IndexUtils::isBasic(fv.index());
}

bool FValUtils::isArithmetic(const FVal & fv)
{
	return IndexUtils::isArithmetic(fv.index());
}

bool FValUtils::isValue(const FVal & fv)
{
	return IndexUtils::isValue(fv.index());
}

FoxType FValUtils::FValToFoxType(const FVal & fv)
{
	return FoxType(fv.index());
}

std::string FValUtils::dumpFVal(const FVal & fv)
{
	std::stringstream output;
	if (std::holds_alternative<NullType>(fv))
		output << "Type : VOID (null)";
	else if (std::holds_alternative<var::varRef>(fv))
	{
		auto vattr = std::get<var::varRef>(fv);
		output << "Type : fvRef, Value:" << vattr.getName();
	}
	else if (std::holds_alternative<IntType>(fv))
		output << "Type : INT, Value : " << std::get<IntType>(fv);
	else if (std::holds_alternative<float>(fv))
		output << "Type : FLOAT, Value : " << std::get<float>(fv);
	else if (std::holds_alternative<std::string>(fv))
		output << "Type : STRING, Value : \"" << std::get<std::string>(fv) << "\"";
	else if (std::holds_alternative<bool>(fv))
	{
		const bool v = std::get<bool>(fv);
		output << "Type : BOOL, Value : " << (v ? "true" : "false");
	}
	else if (std::holds_alternative<CharType>(fv))
	{
		CharType x = std::get<CharType>(fv);
		UTF8::StringManipulator u8sm;
		output << "Type : CHAR, Value : " << x << " = '" << u8sm.wcharToStr(x) << "'";
	}
	else
		throw std::logic_error("Illegal fviant.");
	return output.str();
}

FVal FValUtils::getSampleFValForIndex(const std::size_t & t)
{
	switch (t)
	{
		case TypeIndex::Null_Type:
			return FVal();
		case TypeIndex::basic_Int:
			return FVal((IntType)0);
		case TypeIndex::basic_Float:
			return FVal((float)0.0f);
		case TypeIndex::basic_Char:
			return FVal((CharType)0);
		case TypeIndex::basic_String:
			return FVal(std::string(""));
		case TypeIndex::basic_Bool:
			return FVal((bool)false);
		case TypeIndex::VarRef:
			return FVal(var::varattr());
		case TypeIndex::InvalidIndex:
			throw std::logic_error("Tried to get a sample FVal with an invalid index");
			return FVal();
		default:
			throw std::logic_error("Defaulted while attempting to return a sample FVal for an index. -> Unknown index. Unimplemented type?");
			return FVal();
	}
}

std::string FValUtils::getFValTypeName(const FVal & t)
{
	return FValToFoxType(t).getTypeName();
}

std::string FValUtils::getTypenameForIndex(const std::size_t & ind)
{
	auto searchres = kBuiltinTypes_dict.find(ind);
	if (searchres != kBuiltinTypes_dict.end())
		return searchres->second;
	else
		throw std::invalid_argument("Unknown index in dictionary");
}
