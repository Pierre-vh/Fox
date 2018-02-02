////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Types.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Types.h"

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
		throw std::logic_error("Illegal variant.");
	return output.str();
}
std::string fv_util::dumpVAttr(const var::varattr & var)
{
	std::stringstream output;
	output << "[name:\"" << var.name_ << "\" "
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
		case fval_null:
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
		case fval_varRef:
			return FVal(var::varattr());
		case invalid_index:
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

bool Moonshot::fv_util::isValue(const std::size_t & t)
{
	return isBasic(t) || (t == fval_varRef);
}

bool fv_util::canAssign(Context& context_,const std::size_t & lhs, const std::size_t & rhs)
{
	if ((rhs == fval_null) || (lhs == fval_null))
	{
		context_.reportError("Can't assign a void expression to a variable.");
		return false;
	}
	if (!isBasic(lhs) || !isBasic(rhs))
		// If one of the types isn't basic, no assignement possible.
		return false;
	else if (lhs == rhs) // same type to same type = ok.
		return true;
	// From here, we know lhs and rhs are different.
	else if (!isArithmetic(lhs) || !isArithmetic(rhs)) // one of them is a string
	{
		context_.reportError("Can't assign a string to an arithmetic type and vice versa.");
		return false;
	}
	// Else, we're good, return true.
	return true;
}
bool fv_util::canCastTo(const std::size_t & goal, const std::size_t & basetype)
{
	if (!isBasic(basetype))
		return false;
	if (isArithmetic(basetype)) // base type is arithmetic
		return true; // arithmetic type -> basic type, allowed (and at this point we know it's a basic type due to 1st condition)
	else if (basetype == fval_str) // base type is a string
		return goal == fval_str; // Strings can only be converted to strings (useless convertion anyway).
	return false;
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
		throw std::logic_error("Can't return the biggest of two types when one of the two type isn't arithmetic.");
	return invalid_index;
}

std::size_t fv_util::typeKWtoSizeT(const keywordType & kw)
{
	auto it = kTypeKwToIndex_dict.find(kw);
	if (it != kTypeKwToIndex_dict.end())
		return it->second;
	else
		return invalid_index;
}
// varRef
var::varRef::varRef(const std::string & vname)
{
	name = vname;
}

std::string var::varRef::getName() const
{
	return name;
}

void var::varRef::setName(const std::string & newname)
{
	name = newname;
}

var::varRef::operator bool() const
{
	return (name != "");
}

// varattr
var::varattr::varattr()
{
}

var::varattr::varattr(const std::string & nm)
{
	name_ = nm; // Create a "dummy",unusable varattr with only a name.
}

var::varattr::varattr(const std::string & nm, const std::size_t & ty, const bool & isK) : name_(nm), type(ty), isConst(isK)
{
	wasInit_ = true;
}

var::varattr::operator bool() const
{
	return (wasInit_ && (type != fv_util::fval_null) && (type != fv_util::invalid_index));
}

var::varRef var::varattr::createRef() const
{
	return varRef(name_);
}
