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

// Decl name

ASTNamedDecl::ASTNamedDecl(IdentifierInfo * name) : declName_(name)
{

}

IdentifierInfo * ASTNamedDecl::getDeclName() const
{
	return declName_;;
}

void ASTNamedDecl::setDeclName(IdentifierInfo * nname)
{
	declName_ = nname;
}


// Function arg
ASTArgDecl::ASTArgDecl(IdentifierInfo* id, const QualType & argType) : ASTNamedDecl(id), ty_(argType)
{

}

QualType ASTArgDecl::getType() const
{
	return ty_;
}

void ASTArgDecl::setType(const QualType & qt)
{
	ty_ = qt;
}

void ASTArgDecl::accept(Moonshot::IVisitor &vis)
{
	vis.visit(*this);
}

// Function Declaration
ASTFunctionDecl::ASTFunctionDecl(const Type* returnType, IdentifierInfo* fnId, std::unique_ptr<ASTCompoundStmt> funcbody) :
	returnType_(returnType), ASTNamedDecl(fnId), body_(std::move(funcbody))
{

}

void ASTFunctionDecl::accept(IVisitor & vis)
{
	vis.visit(*this);
}

void ASTFunctionDecl::setReturnType(const Type* ty)
{
	assert(ty && "Type cannot be null!");
	returnType_ = std::move(ty);
}

const Type* ASTFunctionDecl::getReturnType() const
{
	return returnType_;
}

ASTCompoundStmt * ASTFunctionDecl::getBody()
{
	return body_.get();
}

// Function Declaration
void ASTFunctionDecl::setBody(std::unique_ptr<ASTCompoundStmt> arg)
{
	body_ = std::move(arg);
}

const ASTArgDecl* ASTFunctionDecl::getArg(const std::size_t & ind) const
{
	if (ind >= args_.size())
		throw std::out_of_range("Arg does not exists");

	return args_[ind].get();
}

void ASTFunctionDecl::addArg(std::unique_ptr<ASTArgDecl> arg)
{
	args_.emplace_back(std::move(arg));
}

std::size_t ASTFunctionDecl::argsSize() const
{
	return args_.size();
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

QualType ASTVarDecl::getType() const
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

void ASTVarDecl::setType(const QualType &ty)
{
	varTy_ = ty;
}

void ASTVarDecl::setInitExpr(std::unique_ptr<ASTExpr> expr)
{
	if(expr)
		initExpr_ = std::move(expr);
}