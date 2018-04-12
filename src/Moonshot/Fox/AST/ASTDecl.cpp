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
#include <cassert>

using namespace Moonshot;

FunctionArg::FunctionArg(const std::string & argName, const QualType & argType) : name_(argName),ty_(argType)
{

}

std::string FunctionArg::getArgName() const
{
	return name_;
}

void FunctionArg::setArgName(const std::string & name)
{
	name_ = name;
}

QualType FunctionArg::getQualType() const
{
	return ty_;
}

void FunctionArg::setQualType(const QualType & qt)
{
	ty_ = qt;
}

ASTFunctionDecl::ASTFunctionDecl(Type* returnType, const std::string& name, std::vector<FunctionArg> args, std::unique_ptr<ASTCompoundStmt> funcbody)
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

Type* ASTFunctionDecl::getReturnType()
{
	return returnType_;
}

std::string ASTFunctionDecl::getName() const
{
	return name_;
}

FunctionArg ASTFunctionDecl::getArg(const std::size_t & ind) const
{
	return args_[ind];
}

ASTCompoundStmt * ASTFunctionDecl::getBody()
{
	return body_.get();
}

void ASTFunctionDecl::setReturnType(Type *ty)
{
	assert(ty && "Type cannot be null!");
	returnType_ = ty;
}

void ASTFunctionDecl::setName(const std::string & str)
{
	name_ = str;
}

void ASTFunctionDecl::setArgs(const std::vector<FunctionArg>& vec)
{
	args_ = vec;
}

void ASTFunctionDecl::addArg(const FunctionArg & arg)
{
	args_.push_back(arg);
}

void ASTFunctionDecl::setBody(std::unique_ptr<ASTCompoundStmt> arg)
{
	body_ = std::move(arg);
}

ASTFunctionDecl::argIter ASTFunctionDecl::args_begin()
{
	return args_.begin();
}

ASTFunctionDecl::argIter_const ASTFunctionDecl::args_begin() const
{
	return args_.begin();
}

ASTFunctionDecl::argIter ASTFunctionDecl::args_end()
{
	return args_.end();
}

ASTFunctionDecl::argIter_const ASTFunctionDecl::args_end() const
{
	return args_.end();
}

// VarDecl
ASTVarDecl::ASTVarDecl(const std::string& varname,const QualType& ty, std::unique_ptr<ASTExpr> iExpr) : varName_(varname), varTy_(ty)
{
	if (iExpr)
		initExpr_ = std::move(iExpr);
}

void ASTVarDecl::accept(IVisitor& vis)
{
	vis.visit(*this);
}

QualType ASTVarDecl::getVarTy()
{
	return varTy_;
}

ASTExpr * ASTVarDecl::getInitExpr()
{
	return initExpr_.get();
}

bool ASTVarDecl::hasInitExpr() const
{
	return (bool)initExpr_;
}

std::string ASTVarDecl::getVarName() const
{
	return varName_;
}

void ASTVarDecl::setVarName(const std::string & name)
{
	varName_ = name;
}

void ASTVarDecl::setVarType(const QualType &ty)
{
	varTy_ = ty;
}

void ASTVarDecl::setInitExpr(std::unique_ptr<ASTExpr> expr)
{
	if(expr)
		initExpr_ = std::move(expr);
}
