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
#include "Fox/AST/ConstraintVisitor.hpp"
#include "Fox/AST/Type.hpp"
#include <sstream>

using namespace fox;

// ConstraintPrinter, for printing constraints
namespace
{
	class ConstraintPrinter : public ConstraintVisitor<ConstraintPrinter, std::string>
	{
		public:
			ConstraintPrinter()
			{

			}

			std::string visitArrayCS(Constraint*)
			{
				return "ArrayCS";
			}
	};
}

// Base constraint class.

Constraint::Kind Constraint::getKind() const
{
	return kind_;
}

bool Constraint::is(Kind k) const
{
	return kind_ == k;
}

std::string Constraint::toDebugString() const
{
	return ConstraintPrinter().visit(const_cast<Constraint*>(this));
}

void* Constraint::operator new(std::size_t sz, ASTContext& ctxt, std::uint8_t align)
{
	return ctxt.getCSAllocator().allocate(sz, align);
}

Constraint::Constraint(Kind kind): 
	kind_(kind)
{

}

Constraint* Constraint::createArrayCS(ASTContext& ctxt)
{
	if(!ctxt.theArrayCS)
		ctxt.theArrayCS = new(ctxt) Constraint(Kind::ArrayCS);
	return ctxt.theArrayCS;
}