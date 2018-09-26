#include "..\..\includes\Fox\AST\Type.hpp"
#include "..\..\includes\Fox\AST\Type.hpp"
#include "..\..\includes\Fox\AST\Type.hpp"
////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Type.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Fox/AST/Type.hpp"

using namespace fox;

// Type

Type::Type(TypeBase* ty) :
	ty_(ty)
{

}

TypeBase* Type::getPtr()
{
	return ty_;
}

const TypeBase* Type::getPtr() const
{
	return ty_;
}

bool Type::isNull() const
{
	return (ty_ == nullptr);
}

TypeBase* Type::operator->()
{
	assert(ty_ && "Cannot use this if the pointer is null.");
	return ty_;
}

const TypeBase* Type::operator->() const
{
	assert(ty_ && "Cannot use this if the pointer is null.");
	return ty_;
}

Type::operator bool() const
{
	return !isNull();
}

// TypeLoc

TypeLoc::TypeLoc(TypeBase* ty, SourceRange range):
	Type(ty), range_(range)
{

}

TypeLoc::TypeLoc(Type ty, SourceRange range) :
	Type(ty), range_(range)
{

}

SourceRange TypeLoc::getRange() const
{
	return range_;
}

Type TypeLoc::withoutLoc()
{
	return Type(getPtr());
}

const Type TypeLoc::withoutLoc() const
{
	// Remove the const attribute. It's meaningless since
	// we're returning a const Type and a const Type will always
	// return const pointers. This allows us to create the type.
	return Type(const_cast<TypeBase*>(getPtr()));
}
