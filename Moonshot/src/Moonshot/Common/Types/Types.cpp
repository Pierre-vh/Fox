////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Types.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Types.hpp"

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

var::varattr::varattr(const std::string & nm, const std::size_t & ty, const bool & isK) : name_(nm), type_(ty), isConst(isK)
{
	wasInit_ = true;
}

var::varattr::operator bool() const
{
	return (wasInit_ && (type_ != TypeUtils::indexes::fval_null) && (type_ != TypeUtils::indexes::invalid_index));
}

var::varRef var::varattr::createRef() const
{
	return varRef(name_);
}
