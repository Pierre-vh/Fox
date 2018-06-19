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
#include "Stmt.hpp"
#include "Moonshot/Fox/Common/SourceManager.hpp"

#include <sstream>
#include <cassert>

using namespace Moonshot;

// Decl
Decl::Decl(const DeclKind & dkind,const SourceLoc& begLoc, const SourceLoc& endLoc)
	: kind_(dkind), begLoc_(begLoc), endLoc_(endLoc)
{

}

DeclKind Decl::getKind() const
{
	return kind_;
}

SourceLoc Decl::getBegLoc() const
{
	return begLoc_;
}

SourceLoc Decl::getEndLoc() const
{
	return endLoc_;
}

SourceRange Decl::getRange() const
{
	return SourceRange(begLoc_, endLoc_);
}

bool Decl::isLocationAvailable() const
{
	return begLoc_ && endLoc_;
}

bool Decl::isBegLocSet() const
{
	return begLoc_;
}

bool Decl::isEndLocSet() const
{
	return endLoc_;
}

void Decl::setBegLoc(const SourceLoc & loc)
{
	begLoc_ = loc;
}

void Decl::setEndLoc(const SourceLoc & loc)
{
	endLoc_ = loc;
}

// NamedDecl
NamedDecl::NamedDecl(const DeclKind& dkind, IdentifierInfo * name, const SourceLoc& begLoc, const SourceLoc& endLoc)
	: Decl(dkind,begLoc,endLoc), identifier_(name)
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
ArgDecl::ArgDecl(IdentifierInfo* id, const QualType& argType, const SourceLoc& begLoc, const SourceLoc& endLoc)
	: NamedDecl(DeclKind::ArgDecl,id,begLoc,endLoc), ty_(argType)
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

bool ArgDecl::isComplete() const
{
	// Node is valid if it has a identifier, a valid type and a valid loc info
	return this->hasIdentifier() && ty_ && isLocationAvailable();
}

// Function Declaration
FunctionDecl::FunctionDecl(): NamedDecl(DeclKind::FunctionDecl,nullptr,SourceLoc(),SourceLoc())
{

}

FunctionDecl::FunctionDecl(Type* returnType, IdentifierInfo* fnId, std::unique_ptr<CompoundStmt> funcbody,const SourceLoc& begLoc, const SourceLoc& declEndLoc)
	: returnType_(returnType), NamedDecl(DeclKind::FunctionDecl,fnId,begLoc,declEndLoc), body_(std::move(funcbody))
{

}

bool FunctionDecl::isComplete() const
{
	// Every arg must be valid
	for (auto it = args_begin(); it != args_end(); it++)
	{
		if (!it->isComplete())
			return false;
	}
	// and the node must have a body, a return type and an identifier and valid loc info
	return returnType_ && body_ && this->hasIdentifier() && isLocationAvailable();
}

void FunctionDecl::setReturnType(Type* ty)
{
	assert(ty && "Type cannot be null!");
	returnType_ = ty;
}

Type* FunctionDecl::getReturnType()
{
	return returnType_;
}

const Type* FunctionDecl::getReturnType() const
{
	return returnType_;
}

CompoundStmt * FunctionDecl::getBody()
{
	return body_.get();
}

const CompoundStmt* FunctionDecl::getBody() const
{
	return body_.get();
}

void FunctionDecl::setBody(std::unique_ptr<CompoundStmt> arg)
{
	body_ = std::move(arg);
}

ArgDecl* FunctionDecl::getArg(const std::size_t & ind)
{
	assert(ind >= args_.size() && "out of range");
	return args_[ind].get();
}

const ArgDecl* FunctionDecl::getArg(const std::size_t & ind) const
{
	assert(ind >= args_.size() && "out of range");
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
VarDecl::VarDecl(IdentifierInfo * varId,const QualType& ty, std::unique_ptr<Expr> iExpr, const SourceLoc& begLoc, const SourceLoc& endLoc) :
	NamedDecl(DeclKind::VarDecl, varId, begLoc, endLoc), varTy_(ty)
{
	if (iExpr)
		initExpr_ = std::move(iExpr);
}

bool VarDecl::isComplete() const
{
	// must have a type, and id + valid loc info to be considered valid.
	return this->hasIdentifier() && varTy_ && isLocationAvailable();
}

QualType VarDecl::getType() const
{
	return varTy_;
}

Expr* VarDecl::getInitExpr()
{
	return initExpr_.get();
}

const Expr* VarDecl::getInitExpr() const
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
UnitDecl::UnitDecl(IdentifierInfo * id,const FileID& fid)
	: NamedDecl(DeclKind::UnitDecl,id,SourceLoc(),SourceLoc()), fid_(fid)
{
	// NamedDecl constructor is given invalid SourceLocs, because the SourceLocs are updated automatically when a new Decl is Added.
}

void UnitDecl::addDecl(std::unique_ptr<Decl> decl)
{
	assert(decl->getBegLoc().isValid() && decl->getEndLoc().isValid() && "Cannot add an incomplete decl to a unit");

	// Update locs
	if (!isBegLocSet())
		setBegLoc(decl->getBegLoc());

	setEndLoc(decl->getEndLoc());

	decls_.emplace_back(std::move(decl));
	
}

Decl* UnitDecl::getDecl(const std::size_t& idx)
{
	assert(idx < decls_.size() && "out of range");
	return decls_[idx].get();
}

const Decl* UnitDecl::getDecl(const std::size_t& idx) const
{
	assert(idx < decls_.size() && "out of range");
	return decls_[idx].get();
}

std::size_t UnitDecl::getDeclCount() const
{
	return decls_.size();
}

bool UnitDecl::isComplete() const
{
	// Valid if decl number >0 && has an identifier
	return decls_.size() && this->hasIdentifier();
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

FileID UnitDecl::getFileID() const
{
	return fid_;
}

void UnitDecl::setFileID(const FileID& fid)
{
	fid_ = fid;
}