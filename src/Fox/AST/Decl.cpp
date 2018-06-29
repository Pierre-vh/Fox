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
#include "Fox/Common/Source.hpp"

#include <sstream>
#include <cassert>

using namespace fox;

// Decl
Decl::Decl(const DeclKind & kind,const SourceLoc& begLoc, const SourceLoc& endLoc)
	: kind_(kind), begLoc_(begLoc), endLoc_(endLoc)
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

bool Decl::hasLocInfo() const
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
NamedDecl::NamedDecl(const DeclKind& kind, IdentifierInfo * id, const SourceLoc& begLoc, const SourceLoc& endLoc)
	: Decl(kind,begLoc,endLoc), identifier_(id)
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

// Argument Declaration
ArgDecl::ArgDecl() : ArgDecl(nullptr,QualType(),SourceLoc(),SourceRange(),SourceLoc())
{

}

ArgDecl::ArgDecl(IdentifierInfo* id, const QualType& type, const SourceLoc& begLoc, const SourceRange& tyRange, const SourceLoc& endLoc)
	: NamedDecl(DeclKind::ArgDecl,id,begLoc,endLoc), type_(type), tyRange_(tyRange)
{

}

SourceRange ArgDecl::getTypeRange() const
{
	return tyRange_;
}

QualType ArgDecl::getType() const
{
	return type_;
}

void ArgDecl::setType(const QualType & qt)
{
	type_ = qt;
}

bool ArgDecl::isComplete() const
{
	// Node is valid if it has a identifier, a valid type and a valid loc info
	return this->hasIdentifier() && type_ && hasLocInfo();
}

// Function Declaration
FunctionDecl::FunctionDecl(): FunctionDecl(nullptr,nullptr,nullptr,SourceLoc(),SourceLoc(),SourceLoc())
{

}

FunctionDecl::FunctionDecl(Type* returnType, IdentifierInfo* fnId, std::unique_ptr<CompoundStmt> body,const SourceLoc& begLoc, const SourceLoc& headerEndLoc, const SourceLoc& endLoc)
	: NamedDecl(DeclKind::FunctionDecl,fnId,begLoc,endLoc), headEndLoc_(headerEndLoc), body_(std::move(body)), returnType_(returnType)
{

}

void FunctionDecl::setSourceLocs(const SourceLoc& beg, const SourceLoc& declEnd, const SourceLoc& end)
{
	setBegLoc(beg);
	setHeaderEndLoc(declEnd);
	setEndLoc(end);
}

void FunctionDecl::setHeaderEndLoc(const SourceLoc& loc)
{
	headEndLoc_ = loc;
}

SourceLoc FunctionDecl::getHeaderEndLoc() const
{
	return headEndLoc_;
}

SourceRange FunctionDecl::getHeaderRange() const
{
	return SourceRange(getBegLoc(), headEndLoc_);
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
	return returnType_ && body_ && this->hasIdentifier() && hasLocInfo();
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
	assert(ind < args_.size() && "out of range");
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
VarDecl::VarDecl() : VarDecl(nullptr,QualType(),nullptr,SourceLoc(),SourceRange(),SourceLoc())
{

}

VarDecl::VarDecl(IdentifierInfo * id, const QualType& type, std::unique_ptr<Expr> initializer, const SourceLoc& begLoc, const SourceRange& tyRange, const SourceLoc& endLoc) :
	NamedDecl(DeclKind::VarDecl, id, begLoc, endLoc), type_(type), typeRange_(tyRange), initializer_(std::move(initializer))
{

}

bool VarDecl::isComplete() const
{
	// must have a type, and id + valid loc info to be considered valid.
	return this->hasIdentifier() && type_ && hasLocInfo();
}

SourceRange VarDecl::getTypeRange() const
{
	return typeRange_;
}

QualType VarDecl::getType() const
{
	return type_;
}

Expr* VarDecl::getInitExpr()
{
	return initializer_.get();
}

const Expr* VarDecl::getInitExpr() const
{
	return initializer_.get();
}

bool VarDecl::hasInitExpr() const
{
	return (bool)initializer_;
}

void VarDecl::setType(const QualType &ty)
{
	type_ = ty;
}

void VarDecl::setInitExpr(std::unique_ptr<Expr> expr)
{
	if(expr)
		initializer_ = std::move(expr);
}

// ASTUnit
UnitDecl::UnitDecl(IdentifierInfo * id,const FileID& inFile)
	: NamedDecl(DeclKind::UnitDecl,id,SourceLoc(),SourceLoc()), file_(inFile)
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
	return file_;
}

void UnitDecl::setFileID(const FileID& fid)
{
	file_ = fid;
}