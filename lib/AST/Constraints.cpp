////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Constraint.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Fox/AST/Constraints.hpp"
#include "Fox/AST/ASTContext.hpp"

using namespace fox;

// Base constraint class.

Constraint::Kind Constraint::getKind() const
{
	return kind_;
}

void* Constraint::operator new(std::size_t sz, ASTContext& ctxt, std::uint8_t align)
{
	return ctxt.getCSAllocator().allocate(sz, align);
}

Constraint::Constraint(Kind kind): 
	kind_(kind)
{

}

// EqualityCS

EqualityCS::EqualityCS(Type& type):
	Constraint(Kind::EqualityCS), type_(type)
{

}

EqualityCS* EqualityCS::create(ASTContext& ctxt, Type& type)
{
	return new(ctxt) EqualityCS(type);
}

Type& EqualityCS::getType()
{
	return type_;
}

const Type& EqualityCS::getType() const
{
	return type_;
}

ArrayCS* ArrayCS::create(ASTContext& ctxt)
{
	// As an optimization measure, the ArrayCS is unique,
	// like PrimitiveTypes. This is however a "hidden" optimization
	// which isn't guaranteed (might be removed in the future)
	// so we still use the "create" name.
	if(!ctxt.theArrayCS)
		ctxt.theArrayCS = new(ctxt) ArrayCS();
	return ctxt.theArrayCS;
}

ArrayCS::ArrayCS():
	Constraint(Kind::ArrayCS)
{
}
