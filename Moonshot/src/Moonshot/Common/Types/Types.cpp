////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : TypeIndex.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Types.hpp"
#include "FVTypeTraits.hpp"
#include "FoxValueUtils.hpp"
#include "TypesUtils.hpp"
#include <sstream> // std::stringstream

using namespace Moonshot;

// FoxVariableRef
FoxVariableRef::FoxVariableRef(const std::string & vname)
{
	name_ = vname;
}

std::string FoxVariableRef::getName() const
{
	return name_;
}

void FoxVariableRef::setName(const std::string & newname)
{
	name_ = newname;
}

FoxVariableRef::operator bool() const
{
	return (name_ != "");
}

// FoxVariableAttr
FoxVariableAttr::FoxVariableAttr()
{
}

FoxVariableAttr::FoxVariableAttr(const std::string & nm)
{
	name_ = nm; // Create a "dummy",unusable FoxVariableAttr with only a name.
}

FoxVariableAttr::FoxVariableAttr(const std::string & nm, const FoxType & ty) : name_(nm), type_(ty)
{
	wasInit_ = true;
}

FoxVariableAttr::operator bool() const
{
	return (wasInit_ && (type_ != TypeIndex::Void_Type) && (type_ != TypeIndex::InvalidIndex));
}

FoxVariableRef FoxVariableAttr::createRef() const
{
	return FoxVariableRef(name_);
}

std::string FoxVariableAttr::dump() const
{
	std::stringstream output;
	output << "[name:\"" << name_ << "\" type:" << type_.getTypeName() << "]";
	return output.str();
}

// FoxType
FoxType::FoxType(const std::size_t & basicIndex,const bool& isConstant)
{
	isconst_ = isConstant;
	setType(basicIndex);
}

bool FoxType::isBasic() const
{
	return IndexUtils::isBasic(type_index_);
}

bool FoxType::isArithmetic() const
{
	return IndexUtils::isArithmetic(type_index_);
}

bool FoxType::isConst() const
{
	return isconst_;
}

bool FoxType::is(const std::size_t & basicindex) const
{
	return type_index_ == basicindex;;
}

void FoxType::setType(const std::size_t & basicIndex)
{
	type_index_ = basicIndex;
}

void FoxType::setConstAttribute(const bool & val)
{
	isconst_ = val;
}

std::size_t FoxType::getTypeIndex() const
{
	return type_index_;
}

std::string FoxType::getTypeName() const
{
	return (isconst_ ? "CONST " : "") + FValUtils::getTypenameForIndex(type_index_);
}

bool FoxType::compareWith_permissive(const FoxType & other) const
{
	return (type_index_ == other.type_index_);
}

bool FoxType::compareWith_strict(const FoxType & other) const
{
	return compareWith_permissive(other) && (isconst_ == other.isconst_);
}

FoxType & FoxType::operator=(const std::size_t & basicIndex)
{
	setType(basicIndex);
	return *this;
}

bool FoxType::operator==(const std::size_t & basicIndex) const
{
	return type_index_ == basicIndex;
}

bool FoxType::operator==(const FoxType & other) const
{
	return compareWith_strict(other);
}

bool FoxType::operator!=(const std::size_t & basicIndex) const
{
	return !(*this == basicIndex);
}

bool FoxType::operator!=(const FoxType & other) const
{
	return !(*this == other);
}
