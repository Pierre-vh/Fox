#include "Types.h"

using namespace Moonshot;

std::string fv_util::dumpFVal(const FVal & var)
{
	std::stringstream output;
	if (std::holds_alternative<FVAL_NULLTYPE>(var))
		output << "Type : VOID (null)";
	else if (std::holds_alternative<var::varattr>(var))
	{
		auto vattr = std::get<var::varattr>(var);
		output << "Type : varattr, Value:";
		output << dumpVAttr(vattr);
	}
	else if (std::holds_alternative<int>(var))
		output << "Type : INT, Value : " << std::get<int>(var);
	else if (std::holds_alternative<float>(var))
		output << "Type : FLOAT, Value : " << std::get<float>(var);
	else if (std::holds_alternative<std::string>(var))
		output << "Type : STRING, Value : \"" << std::get<std::string>(var) << "\"";
	else if (std::holds_alternative<bool>(var))
	{
		bool v = std::get<bool>(var);
		output << "Type : BOOL, Value : " << (v ? "true" : "false");
	}
	else if (std::holds_alternative<char>(var))
	{
		char x = std::get<char>(var);
		output << "Type : CHAR, Value : " << (int)x <<  " = '" << x << "'";
	}
	else
		E_CRITICAL("Illegal variant.");
	return output.str();
}
std::string fv_util::dumpVAttr(const var::varattr & var)
{
	std::stringstream output;
	output << "[name:\"" << var.name << "\" "
		<< "type: " << (var.isConst ? "CONST " : "");
	auto friendlyname = kType_dict.find(var.type);
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
		case fval_void:
			return FVal();
		case fval_int:
			return FVal((int)0);
		case fval_float:
			return FVal((float)0.0f);
		case fval_char:
			return FVal((char)0);
		case fval_str:
			return FVal(std::string(""));
		case fval_bool:
			return FVal((bool)false);
		case fval_vattr:
			return FVal(var::varattr());
		case invalid_index:
			E_CRITICAL("Tried to get a sample FVal with an invalid index");
			return FVal();
		default:
			E_CRITICAL("Defaulted while attempting to return a sample FVal for an index. -> Unknown index. Unimplemented type?");
			return FVal();
	}
}

std::string fv_util::indexToTypeName(const std::size_t & t)
{
	auto a = kType_dict.find(t);
	return (a != kType_dict.end()) ? a->second : "!IMPOSSIBLE_TYPE!";
}

bool fv_util::isBasic(const std::size_t & t)
{
	switch (t)
	{
		case fval_int:
		case fval_float:
		case fval_char:
		case fval_bool:
		case fval_str:
			return true;
		default:
			return false;
	}
}

bool fv_util::isArithmetic(const std::size_t & t)
{
	return (isBasic(t) && (t != fval_str));
}

bool fv_util::canAssign(const std::size_t & lhs, const std::size_t & rhs)
{
	if ((rhs == fval_void) || (lhs == fval_void))
	{
		E_ERROR("[TYPECHECK] Can't assign a void expression to a variable.");
		return false;
	}
	if (!isBasic(lhs) || !isBasic(rhs))
		// If one of the types isn't basic, no assignement possible.
		return false;
	else if (lhs == rhs) // same type to same type = ok.
		return true;
	// From here, we know lhs and rhs are different.
	else if (!isArithmetic(lhs) || isArithmetic(rhs)) // one of them is a string, and the other isn't
	{
		E_ERROR("[TYPECHECK] Can't assign a string to an arithmetic type and vice versa.");
		return false;
	}
	// Else, we're good, return true.
	return true;
}

std::size_t fv_util::getBiggest(const std::size_t & lhs, const std::size_t & rhs)
{
	if (isArithmetic(lhs) && isArithmetic(rhs))
	{
		if ((lhs == fval_float) || (rhs == fval_float))
			return fval_float;
		else if ((lhs == fval_int) || (rhs == fval_int))
			return fval_int;
		else if ((lhs == fval_char) || (rhs == fval_char))
			return fval_char;
		else
			return fval_bool;
	}
	else
	
		E_CRITICAL("Can't return the biggest of two types when one of the two type isn't arithmetic.");
	return invalid_index;
}

std::size_t fv_util::typeKWtoSizeT(const lex::keywords & kw)
{
	auto it = kTypeKwToIndex_dict.find(kw);
	if (it != kTypeKwToIndex_dict.end())
		return it->second;
	else
		return invalid_index;
}

std::string fv_util::indexToStr(const std::size_t & index)
{
	auto it = kType_dict.find(index);
	if (it != kType_dict.end())
		return it->second;
	else
		return "<UNKNOWN>";
}

// varattr

var::varattr::varattr()
{
}

var::varattr::varattr(const std::string & nm)
{
	name = nm; // Create a "dummy",unusable varattr with only a name.
}

var::varattr::varattr(const std::string & nm, const std::size_t & ty, const bool & isK) : name(nm), type(ty), isConst(isK)
{
	wasInit_ = true;
}

var::varattr::operator bool() const
{
	return (wasInit_ && (type != fv_util::fval_void) && (type != fv_util::invalid_index));
}
