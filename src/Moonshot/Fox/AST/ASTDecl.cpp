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

FunctionArg::FunctionArg(IdentifierInfo* id, const QualType & argType) : id_(id),ty_(argType)
{

}

IdentifierInfo* FunctionArg::getArgIdentifier()
{
	return id_;
}

void FunctionArg::setArgIdentifier(IdentifierInfo* id)
{
	id_ = id;
}

QualType FunctionArg::getQualType() const
{
	return ty_;
}

void FunctionArg::setQualType(const QualType & qt)
{
	ty_ = qt;
}

ASTFunctionDecl::ASTFunctionDecl(TypePtr returnType, IdentifierInfo* fnId, std::vector<FunctionArg> args, std::unique_ptr<ASTCompoundStmt> funcbody) :
	returnType_(std::move(returnType)), fnId_(fnId), body_(std::move(funcbody)), args_(args)
{

}

void ASTFunctionDecl::accept(IVisitor & vis)
{
	vis.visit(*this);
}

TypePtr ASTFunctionDecl::getReturnType()
{
	return TypePtr(returnType_);
}

void ASTFunctionDecl::setReturnType(TypePtr ty)
{
	assert(ty && "Type cannot be null!");
	returnType_ = std::move(ty);
}

IdentifierInfo* ASTFunctionDecl::getFunctionIdentifier()
{
	return fnId_;
}

void ASTFunctionDecl::setFunctionIdentifier(IdentifierInfo * id)
{
	fnId_ = id;
}

FunctionArg ASTFunctionDecl::getArg(const std::size_t & ind) const
{
	return args_[ind];
}

ASTCompoundStmt * ASTFunctionDecl::getBody()
{
	return body_.get();
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
ASTVarDecl::ASTVarDecl(IdentifierInfo * varId,const QualType& ty, std::unique_ptr<ASTExpr> iExpr) : varId_(varId), varTy_(ty)
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

IdentifierInfo * ASTVarDecl::getVarIdentifier()
{
	return varId_;
}

void ASTVarDecl::setVarIdentifier(IdentifierInfo * varId)
{
	varId_ = varId;
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
