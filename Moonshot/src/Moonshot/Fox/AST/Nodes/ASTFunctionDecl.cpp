////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTFunctionDecl.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "ASTFunctionDecl.hpp"
#include <stdexcept>
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

ASTFunctionDecl::ASTFunctionDecl(const FoxType & returnType, const std::string & name, std::vector<FoxFunctionArg> args, std::unique_ptr<ASTCompoundStmt> funcbody):
	returnType_(returnType),name_(name),args_(args),body_(std::move(funcbody))
{
}
