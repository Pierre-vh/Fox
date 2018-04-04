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

#include "IVisitor.hpp"

#include <sstream>

using namespace Moonshot;

FoxFunctionArg::FoxFunctionArg(const std::string & nm, const std::size_t & ty, const bool &isK, const bool & isref)
{
	name_ = nm;
	type_ = ty;
	type_.setConstAttribute(isK);

	isRef_ = isref;
	wasInit_ = true;
}

bool FoxFunctionArg::isRef() const
{
	return isRef_;
}

void FoxFunctionArg::setIsRef(const bool & nref)
{
	isRef_ = nref;
}

bool FoxFunctionArg::isConst() const
{
	return type_.isConst();
}

void FoxFunctionArg::setConst(const bool & k)
{
	type_.setConstAttribute(k);
}

std::string FoxFunctionArg::dump() const
{
	std::stringstream output;
	output << "Name:\"" << name_ << "\" Type:" << type_.getTypeName() << " isReference:" << (isRef_ ? "Yes" : "No") << "";
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

ASTFunctionDecl::ASTFunctionDecl(const FoxType & returnType, const std::string & name, std::vector<FoxFunctionArg> args, std::unique_ptr<ASTCompoundStmt> funcbody)
{
	setReturnType(returnType);
	setName(name);
	setBody(std::move(funcbody));
	setArgs(args);
}

void ASTFunctionDecl::accept(IVisitor & vis)
{
	vis.visit(*this);
}

FoxType ASTFunctionDecl::getReturnType() const
{
	return returnType_;
}

std::string ASTFunctionDecl::getName() const
{
	return name_;
}

FoxFunctionArg ASTFunctionDecl::getArg(const std::size_t & ind) const
{
	return args_[ind];
}

ASTCompoundStmt * ASTFunctionDecl::getBody()
{
	return body_.get();
}

void ASTFunctionDecl::setReturnType(const FoxType & ft)
{
	returnType_ = ft;
}

void ASTFunctionDecl::setName(const std::string & str)
{
	name_ = str;
}

void ASTFunctionDecl::setArgs(const std::vector<FoxFunctionArg>& vec)
{
	args_ = vec;
}

void ASTFunctionDecl::addArg(const FoxFunctionArg & arg)
{
	args_.push_back(arg);
}

void ASTFunctionDecl::setBody(std::unique_ptr<ASTCompoundStmt> arg)
{
	body_ = std::move(arg);
}

void ASTFunctionDecl::iterateArgs(std::function<void(FoxFunctionArg)> fn)
{
	for (const auto& elem : args_)
		fn(elem);
}

// VarDecl
ASTVarDecl::ASTVarDecl(const FoxVariableAttr & attr, std::unique_ptr<IASTExpr> iExpr)
{
	if (attr)
	{
		vattr_ = attr;
		setInitExpr(std::move(iExpr));
	}
	else
		throw std::invalid_argument("Supplied an empty FoxVariableAttr object to the constructor.");
}

void ASTVarDecl::accept(IVisitor& vis)
{
	vis.visit(*this);
}

FoxVariableAttr ASTVarDecl::getVarAttr() const
{
	return vattr_;
}

IASTExpr * ASTVarDecl::getInitExpr()
{
	return initExpr_.get();
}

bool ASTVarDecl::hasInitExpr() const
{
	return (bool)initExpr_;
}

void ASTVarDecl::setVarAttr(const FoxVariableAttr & vattr)
{
	vattr_ = vattr;
}

void ASTVarDecl::setInitExpr(std::unique_ptr<IASTExpr> expr)
{
	initExpr_ = std::move(expr);
}
