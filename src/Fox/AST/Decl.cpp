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
Decl::Decl(DeclKind kind,const SourceLoc& begLoc, const SourceLoc& endLoc)
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
	return isBegLocSet() && isEndLocSet();
}

bool Decl::isValid() const
{
	return hasLocInfo();
}

bool Decl::isBegLocSet() const
{
	return begLoc_.isValid();
}

bool Decl::isEndLocSet() const
{
	return endLoc_.isValid();
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
NamedDecl::NamedDecl(DeclKind kind, IdentifierInfo * id, const SourceLoc& begLoc, const SourceLoc& endLoc)
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

bool NamedDecl::isValid() const
{
	return Decl::isValid() && hasIdentifier();
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

bool ArgDecl::isValid() const
{
	return NamedDecl::isValid() && type_ && tyRange_;
}

// Function Declaration
FunctionDecl::FunctionDecl(): FunctionDecl(nullptr,nullptr,nullptr,SourceLoc(),SourceLoc(),SourceLoc())
{

}

FunctionDecl::FunctionDecl(Type* returnType, IdentifierInfo* fnId, std::unique_ptr<CompoundStmt> body,const SourceLoc& begLoc, const SourceLoc& headerEndLoc, const SourceLoc& endLoc)
	: NamedDecl(DeclKind::FunctionDecl,fnId,begLoc,endLoc), headEndLoc_(headerEndLoc), body_(std::move(body)), returnType_(returnType)
{
	argsAreValid_ = true;
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

bool FunctionDecl::isValid() const
{
	return NamedDecl::isValid() && body_ && returnType_ && headEndLoc_ && argsAreValid_;
}

void FunctionDecl::setReturnType(Type* ty)
{
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

CompoundStmt* FunctionDecl::getBody()
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

ArgDecl* FunctionDecl::getArg(std::size_t ind)
{
	assert(ind < args_.size() && "out-of-range");
	return args_[ind].get();
}

const ArgDecl* FunctionDecl::getArg(std::size_t ind) const
{
	assert(ind < args_.size() && "out-of-range");
	return args_[ind].get();
}

void FunctionDecl::addArg(std::unique_ptr<ArgDecl> arg)
{
	if (!arg->isValid())
		argsAreValid_ = false;

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

bool VarDecl::isValid() const
{
	return NamedDecl::isValid() && type_ && typeRange_;
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
UnitDecl::UnitDecl(IdentifierInfo * id,FileID inFile)
	: NamedDecl(DeclKind::UnitDecl,id,SourceLoc(),SourceLoc()), file_(inFile)
{
	declsAreValid_ = true;
}

void UnitDecl::addDecl(std::unique_ptr<Decl> decl)
{
	// Update locs
	if (!isBegLocSet())
		setBegLoc(decl->getBegLoc());

	setEndLoc(decl->getEndLoc());

	// Check the decl that we're adding in.
	if (!decl->isValid())
		declsAreValid_ = false;

	decls_.emplace_back(std::move(decl));
}

Decl* UnitDecl::getDecl(std::size_t idx)
{
	assert(idx < decls_.size() && "out-of-range");
	return decls_[idx].get();
}

const Decl* UnitDecl::getDecl(std::size_t idx) const
{
	assert(idx < decls_.size() && "out-of-range");
	return decls_[idx].get();
}

std::size_t UnitDecl::getDeclCount() const
{
	return decls_.size();
}

bool UnitDecl::isValid() const
{
	return file_ && NamedDecl::isValid() && (decls_.size() != 0) && declsAreValid_;
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