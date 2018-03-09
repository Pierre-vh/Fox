////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTDecl.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "ASTDecl.hpp"
#include "ASTExpr.hpp"
#include <sstream>

using namespace Moonshot;

FoxFunctionArg::FoxFunctionArg(const std::string & nm, const std::size_t & ty, const bool isK, const bool & isref)
{
	name_ = nm;
	type_ = ty;
	isRef_ = isref;
	wasInit_ = true;
}

std::string FoxFunctionArg::dump() const
{
	std::stringstream output;
	output << "[name:\"" << name_ << "\" type:" << type_.getTypeName() << " isReference:" << (isRef_ ? "Yes" : "No") << "]";
	return output.str();
}

FoxFunctionArg::operator bool() const
{
	return (wasInit_ && (type_ != TypeIndex::Void_Type) && (type_ != TypeIndex::InvalidIndex));
}

bool FoxFunctionArg::operator==(const FoxFunctionArg & other) const
{
	return (name_ == other.name_) && (type_ == other.type_);
}

bool FoxFunctionArg::operator!=(const FoxFunctionArg & other) const
{
	return !(*this == other);
}

ASTFunctionDecl::ASTFunctionDecl(const FoxType & returnType, const std::string & name, std::vector<FoxFunctionArg> args, std::unique_ptr<ASTCompoundStmt> funcbody) :
	returnType_(returnType), name_(name), args_(args), body_(std::move(funcbody))
{

}

ASTVarDecl::ASTVarDecl(const FoxVariableAttr & attr, std::unique_ptr<IASTExpr> iExpr)
{
	if (attr)
	{
		vattr_ = attr;
		if (iExpr)						// if iexpr is valid, move it to our attribute.
			initExpr_ = std::move(iExpr);
	}
	else
		throw std::invalid_argument("Supplied an empty FoxVariableAttr object to the constructor.");
}

void ASTVarDecl::accept(IVisitor& vis)
{
	vis.visit(*this);
}
