////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Decl.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Expr.hpp"
#include "Decl.hpp"
#include "IVisitor.hpp"

#include <sstream>
#include <cassert>

using namespace Moonshot;

// Decl name

NamedDecl::NamedDecl(IdentifierInfo * name) : identifier_(name)
{

}

IdentifierInfo * NamedDecl::getIdentifier() const
{
	return identifier_;;
}

void NamedDecl::setIdentifier(IdentifierInfo * nname)
{
	identifier_ = nname;
}

bool NamedDecl::hasIdentifier() const
{
	return (bool)identifier_;
}

// Function arg
ArgDecl::ArgDecl(IdentifierInfo* id, const QualType & argType) : NamedDecl(id), ty_(argType)
{

}

QualType ArgDecl::getType() const
{
	return ty_;
}

void ArgDecl::setType(const QualType & qt)
{
	ty_ = qt;
}

void ArgDecl::accept(Moonshot::IVisitor &vis)
{
	vis.visit(*this);
}

bool ArgDecl::isValid()
{
	// Node is valid if it has a identifier_ and a valid type.
	return this->hasIdentifier() && ty_;
}

// Function Declaration
FunctionDecl::FunctionDecl(const Type* returnType, IdentifierInfo* fnId, std::unique_ptr<CompoundStmt> funcbody) :
	returnType_(returnType), NamedDecl(fnId), body_(std::move(funcbody))
{

}

void FunctionDecl::accept(IVisitor & vis)
{
	vis.visit(*this);
}

bool FunctionDecl::isValid()
{
	// must has a body, a return type and an identifier.
	return returnType_ && body_ && this->hasIdentifier();
}

void FunctionDecl::setReturnType(const Type* ty)
{
	assert(ty && "Type cannot be null!");
	returnType_ = std::move(ty);
}

const Type* FunctionDecl::getReturnType() const
{
	return returnType_;
}

CompoundStmt * FunctionDecl::getBody()
{
	return body_.get();
}

// Function Declaration
void FunctionDecl::setBody(std::unique_ptr<CompoundStmt> arg)
{
	body_ = std::move(arg);
}

ArgDecl* FunctionDecl::getArg(const std::size_t & ind)
{
	if (ind >= args_.size())
		throw std::out_of_range("Arg does not exists");

	return args_[ind].get();
}

void FunctionDecl::addArg(std::unique_ptr<ArgDecl> arg)
{
	args_.emplace_back(std::move(arg));
}

std::size_t FunctionDecl::argsSize() const
{
	return args_.size();
}

FunctionDecl::ArgVecIter FunctionDecl::args_begin()
{
	return args_.begin();
}

FunctionDecl::ArgVecConstIter FunctionDecl::args_begin() const
{
	return args_.begin();
}

FunctionDecl::ArgVecIter FunctionDecl::args_end()
{
	return args_.end();
}

FunctionDecl::ArgVecConstIter FunctionDecl::args_end() const
{
	return args_.end();
}

// VarDecl
VarDecl::VarDecl(IdentifierInfo * varId,const QualType& ty, std::unique_ptr<Expr> iExpr) : NamedDecl(varId), varTy_(ty)
{
	if (iExpr)
		initExpr_ = std::move(iExpr);
}

void VarDecl::accept(IVisitor& vis)
{
	vis.visit(*this);
}

bool VarDecl::isValid()
{
	// must have a type and id to be valid.
	return this->hasIdentifier() && varTy_;
}

QualType VarDecl::getType() const
{
	return varTy_;
}

Expr * VarDecl::getInitExpr()
{
	return initExpr_.get();
}

bool VarDecl::hasInitExpr() const
{
	return (bool)initExpr_;
}

void VarDecl::setType(const QualType &ty)
{
	varTy_ = ty;
}

void VarDecl::setInitExpr(std::unique_ptr<Expr> expr)
{
	if(expr)
		initExpr_ = std::move(expr);
}

// ASTUnit
UnitDecl::UnitDecl(IdentifierInfo * id): NamedDecl(id)
{
}

void UnitDecl::addDecl(std::unique_ptr<Decl> decl)
{
	decls_.emplace_back(std::move(decl));
}

Decl * UnitDecl::getDecl(const std::size_t & idx)
{
	if (idx < decls_.size())
		return decls_[idx].get();
	return nullptr;
}

std::size_t UnitDecl::getDeclCount() const
{
	return decls_.size();
}

bool UnitDecl::isValid()
{
	// Valid if decl number >0 && has an identifier
	return decls_.size() && this->hasIdentifier();
}

void UnitDecl::accept(IVisitor &vis)
{
	vis.visit(*this);
}

UnitDecl::DeclVecIter UnitDecl::decls_beg()
{
	return decls_.begin();
}

UnitDecl::DeclVecIter UnitDecl::decls_end()
{
	return decls_.end();
}

UnitDecl::DeclVecConstIter UnitDecl::decls_beg() const
{
	return decls_.begin();
}

UnitDecl::DeclVecConstIter UnitDecl::decls_end() const
{
	return decls_.end();
}

