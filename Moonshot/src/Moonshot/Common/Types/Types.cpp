////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Types.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Types.hpp"
#include "FVTypeTraits.hpp"
#include <sstream> // std::stringstream

using namespace Moonshot;

// varRef
var::varRef::varRef(const std::string & vname)
{
	name_ = vname;
}

std::string var::varRef::getName() const
{
	return name_;
}

void var::varRef::setName(const std::string & newname)
{
	name_ = newname;
}

var::varRef::operator bool() const
{
	return (name_ != "");
}

// varattr
var::varattr::varattr()
{
}

var::varattr::varattr(const std::string & nm)
{
	name_ = nm; // Create a "dummy",unusable varattr with only a name.
}

var::varattr::varattr(const std::string & nm, const std::size_t & ty, const bool & isK) : name_(nm), type_(ty), isConst_(isK)
{
	wasInit_ = true;
}

var::varattr::operator bool() const
{
	return (wasInit_ && (type_ != Types::builtin_Null) && (type_ != Types::InvalidIndex));
}

var::varRef var::varattr::createRef() const
{
	return varRef(name_);
}

// FoxType

FoxType::FoxType(const std::size_t & basicIndex)
{
	setType(basicIndex);
}

bool FoxType::isBuiltin() const
{
	return (builtin_type_index_ != Types::InvalidIndex);
}

bool FoxType::isBasic() const
{
	return TypeUtils::isBasic(builtin_type_index_);
}

void FoxType::setType(const std::size_t & basicIndex)
{
	builtin_type_index_ = basicIndex;
}

std::size_t FoxType::getBuiltInTypeIndex() const
{
	return std::size_t();
}

FoxType & FoxType::operator=(const std::size_t & basicIndex)
{
	setType(basicIndex);
	return *this;
}
