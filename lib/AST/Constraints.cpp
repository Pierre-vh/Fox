////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Constraints.cpp											
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

Type& EqualityCS::getType()
{
	return type_;
}

const Type& EqualityCS::getType() const
{
	return type_;
}

ArrayCS::ArrayCS():
	Constraint(Kind::ArrayCS)
{
}
