#include "TypesUtils.h"

using namespace Moonshot;

std::string fv_util::dumpFVal(const FVal & var)
{
	std::stringstream output;
	if (std::holds_alternative<std::monostate>(var))
		output << "Type : VOID (null)";
	else if (std::holds_alternative<var::varRef>(var))
	{
		auto vattr = std::get<var::varRef>(var);
		output << "Type : varRef, Value:" << vattr.getName();
	}
	else if (std::holds_alternative<IntType>(var))
		output << "Type : INT, Value : " << std::get<IntType>(var);
	else if (std::holds_alternative<float>(var))
		output << "Type : FLOAT, Value : " << std::get<float>(var);
	else if (std::holds_alternative<std::string>(var))
		output << "Type : STRING, Value : \"" << std::get<std::string>(var) << "\"";
	else if (std::holds_alternative<bool>(var))
	{
		bool v = std::get<bool>(var);
		output << "Type : BOOL, Value : " << (v ? "true" : "false");
	}
	else if (std::holds_alternative<CharType>(var))
	{
		CharType x = std::get<CharType>(var);
		UTF8::StringManipulator u8sm;
		output << "Type : CHAR, Value : " << (int32_t)x << " = '" << u8sm.wcharToStr(x) << "'";
	}
	else
		throw std::logic_error("Illegal variant.");
	return output.str();
}
std::string fv_util::dumpVAttr(const var::varattr & var)
{
	std::stringstream output;
	output << "[name:\"" << var.name_ << "\" "
		<< "type: " << (var.isConst ? "CONST " : "");
	auto friendlyname = kType_dict.find(var.type_);
	if (friendlyname != kType_dict.end())
		output << friendlyname->second;
	else
		output << "<UNKNOWN>";
	output << "]";
	return output.str();
}
FVal fv_util::getSampleFValForIndex(const std::size_t & t)
{
	switch (t)
	{
	case indexes::fval_null:
		return FVal();
	case indexes::fval_int:
		return FVal((IntType)0);
	case indexes::fval_float:
		return FVal((float)0.0f);
	case indexes::fval_char:
		return FVal((CharType)0);
	case indexes::fval_str:
		return FVal(std::string(""));
	case indexes::fval_bool:
		return FVal((bool)false);
	case indexes::fval_varRef:
		return FVal(var::varattr());
	case indexes::invalid_index:
		throw std::logic_error("Tried to get a sample FVal with an invalid index");
		return FVal();
	default:
		throw std::logic_error("Defaulted while attempting to return a sample FVal for an index. -> Unknown index. Unimplemented type?");
		return FVal();
	}
}

std::string fv_util::indexToTypeName(const std::size_t & t)
{
	auto a = kType_dict.find(t);
	if (a != kType_dict.end())
		return a->second;
	return "!IMPOSSIBLE_TYPE!";
}

bool fv_util::isBasic(const std::size_t & t)
{
	switch (t)
	{
	case indexes::fval_int:
	case indexes::fval_float:
	case indexes::fval_char:
	case indexes::fval_bool:
	case indexes::fval_str:
		return true;
	default:
		return false;
	}
}

bool fv_util::isArithmetic(const std::size_t & t)
{
	return (isBasic(t) && (t != indexes::fval_str));
}

bool Moonshot::fv_util::isValue(const std::size_t & t)
{
	return isBasic(t) || (t == indexes::fval_varRef);
}

bool fv_util::canAssign(const std::size_t & lhs, const std::size_t & rhs)
{
	if ((rhs == indexes::fval_null) || (lhs == indexes::fval_null))
		return false; // Can't assign a void expression to a variable.
	if (!isBasic(lhs) || !isBasic(rhs))
		// If one of the types isn't basic, no assignement possible.
		return false;
	else if (lhs == rhs) // same type to same type = ok.
		return true;
	// From here, we know lhs and rhs are different.
	else if (!isArithmetic(lhs) || !isArithmetic(rhs)) // one of them is a string
		return false;  // Can't assign a string to an arithmetic type and vice versa.
					   // Else, we're good, return true.
	return true;
}
bool fv_util::canCastTo(const std::size_t & goal, const std::size_t & basetype)
{
	if (!isBasic(basetype))
		return false;
	if (isArithmetic(basetype)) // base type is arithmetic
		return true; // arithmetic type -> basic type, allowed (and at this point we know it's a basic type due to 1st condition)
	else if (basetype == indexes::fval_str) // base type is a string
		return goal == indexes::fval_str; // Strings can only be converted to strings (useless convertion anyway).
	return false;
}

std::size_t fv_util::getBiggest(const std::size_t & lhs, const std::size_t & rhs)
{
	if (isArithmetic(lhs) && isArithmetic(rhs))
	{
		if ((lhs == indexes::fval_float) || (rhs == indexes::fval_float))
			return indexes::fval_float;
		else if ((lhs == indexes::fval_int) || (rhs == indexes::fval_int))
			return indexes::fval_int;
		else if ((lhs == indexes::fval_char) || (rhs == indexes::fval_char))
			return indexes::fval_char;
		else
			return indexes::fval_bool;
	}
	else
		throw std::logic_error("Can't return the biggest of two types when one of the two type isn't arithmetic.");
	return indexes::invalid_index;
}
