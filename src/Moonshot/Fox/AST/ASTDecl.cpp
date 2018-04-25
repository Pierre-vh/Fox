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

bool ASTArgDecl::isValid()
{
	// Node is valid if it has a declName_ and a valid type.
	return declName_ && ty_;
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

bool ASTFunctionDecl::isValid()
{
	// must has a body, a return type and a name.
	return returnType_ && body_ && declName_;
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

ASTArgDecl* ASTFunctionDecl::getArg(const std::size_t & ind)
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

ASTFunctionDecl::ArgVecIter ASTFunctionDecl::args_begin()
{
	return args_.begin();
}

ASTFunctionDecl::ArgVecConstIter ASTFunctionDecl::args_begin() const
{
	return args_.begin();
}

ASTFunctionDecl::ArgVecIter ASTFunctionDecl::args_end()
{
	return args_.end();
}

ASTFunctionDecl::ArgVecConstIter ASTFunctionDecl::args_end() const
{
	return args_.end();
}

// VarDecl
ASTVarDecl::ASTVarDecl(IdentifierInfo * varId,const QualType& ty, std::unique_ptr<ASTExpr> iExpr) : ASTNamedDecl(varId), varTy_(ty)
{
	if (iExpr)
		initExpr_ = std::move(iExpr);
}

void ASTVarDecl::accept(IVisitor& vis)
{
	vis.visit(*this);
}

bool ASTVarDecl::isValid()
{
	// must have a type and name to be valid.
	return declName_ && varTy_;
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

void ASTVarDecl::setType(const QualType &ty)
{
	varTy_ = ty;
}

void ASTVarDecl::setInitExpr(std::unique_ptr<ASTExpr> expr)
{
	if(expr)
		initExpr_ = std::move(expr);
}

// ASTUnit
void ASTUnitDecl::addDecl(std::unique_ptr<ASTDecl> decl)
{
	decls_.emplace_back(std::move(decl));
}

ASTDecl * ASTUnitDecl::getDecl(const std::size_t & idx)
{
	if (idx < decls_.size())
		return decls_[idx].get();
	return nullptr;
}

std::size_t ASTUnitDecl::getDeclCount() const
{
	return decls_.size();
}

bool ASTUnitDecl::isValid()
{
	// Valid if decl number >0
	return decls_.size();
}

void ASTUnitDecl::accept(IVisitor &vis)
{
	vis.visit(*this);
}

ASTUnitDecl::DeclVecIter ASTUnitDecl::decls_beg()
{
	return decls_.begin();
}

ASTUnitDecl::DeclVecIter ASTUnitDecl::decls_end()
{
	return decls_.end();
}

ASTUnitDecl::DeclVecConstIter ASTUnitDecl::decls_beg() const
{
	return decls_.begin();
}

ASTUnitDecl::DeclVecConstIter ASTUnitDecl::decls_end() const
{
	return decls_.end();
}

