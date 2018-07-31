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
#include "ASTContext.hpp"
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

void* Decl::operator new(std::size_t sz, ASTContext& ctxt, std::uint8_t align)
{
	return ctxt.getAllocator().allocate(sz, align);
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
ParamDecl::ParamDecl() : ParamDecl(nullptr,QualType(),SourceLoc(),SourceRange(),SourceLoc())
{

}

ParamDecl::ParamDecl(IdentifierInfo* id, const QualType& type, const SourceLoc& begLoc, const SourceRange& tyRange, const SourceLoc& endLoc)
	: NamedDecl(DeclKind::ParamDecl,id,begLoc,endLoc), type_(type), tyRange_(tyRange)
{

}

SourceRange ParamDecl::getTypeRange() const
{
	return tyRange_;
}

QualType ParamDecl::getType() const
{
	return type_;
}

void ParamDecl::setType(const QualType & qt)
{
	type_ = qt;
}

bool ParamDecl::isValid() const
{
	return NamedDecl::isValid() && type_ && tyRange_;
}

// Function Declaration
FuncDecl::FuncDecl(): FuncDecl(nullptr,nullptr,nullptr,SourceLoc(),SourceLoc(),SourceLoc())
{

}

FuncDecl::FuncDecl(Type* returnType, IdentifierInfo* fnId, CompoundStmt* body,const SourceLoc& begLoc, const SourceLoc& headerEndLoc, const SourceLoc& endLoc)
	: NamedDecl(DeclKind::FuncDecl,fnId,begLoc,endLoc), headEndLoc_(headerEndLoc), body_(body), returnType_(returnType)
{
	paramsAreValid_ = true;
}

void FuncDecl::setSourceLocs(const SourceLoc& beg, const SourceLoc& declEnd, const SourceLoc& end)
{
	setBegLoc(beg);
	setHeaderEndLoc(declEnd);
	setEndLoc(end);
}

void FuncDecl::setHeaderEndLoc(const SourceLoc& loc)
{
	headEndLoc_ = loc;
}

SourceLoc FuncDecl::getHeaderEndLoc() const
{
	return headEndLoc_;
}

SourceRange FuncDecl::getHeaderRange() const
{
	return SourceRange(getBegLoc(), headEndLoc_);
}

bool FuncDecl::isValid() const
{
	return NamedDecl::isValid() && body_ && returnType_ && headEndLoc_ && paramsAreValid_;
}

void FuncDecl::setReturnType(Type* ty)
{
	returnType_ = ty;
}

Type* FuncDecl::getReturnType()
{
	return returnType_;
}

const Type* FuncDecl::getReturnType() const
{
	return returnType_;
}

CompoundStmt* FuncDecl::getBody()
{
	return body_;
}

const CompoundStmt* FuncDecl::getBody() const
{
	return body_;
}

void FuncDecl::setBody(CompoundStmt* body)
{
	body_ = body;
}

ParamDecl* FuncDecl::getParamDecl(std::size_t ind)
{
	assert(ind < params_.size() && "out-of-range");
	return params_[ind];
}

const ParamDecl* FuncDecl::getParamDecl(std::size_t ind) const
{
	assert(ind < params_.size() && "out-of-range");
	return params_[ind];
}

void FuncDecl::addParamDecl(ParamDecl* arg)
{
	if (!arg->isValid())
		paramsAreValid_ = false;

	params_.push_back(arg);
}

std::size_t FuncDecl::getNumParams() const
{
	return params_.size();
}

FuncDecl::ParamVecIter FuncDecl::params_begin()
{
	return params_.begin();
}

FuncDecl::ParamVecConstIter FuncDecl::params_begin() const
{
	return params_.begin();
}

FuncDecl::ParamVecIter FuncDecl::params_end()
{
	return params_.end();
}

FuncDecl::ParamVecConstIter FuncDecl::params_end() const
{
	return params_.end();
}

// VarDecl
VarDecl::VarDecl() : VarDecl(nullptr,QualType(),nullptr,SourceLoc(),SourceRange(),SourceLoc())
{

}

VarDecl::VarDecl(IdentifierInfo * id, const QualType& type, Expr* init, const SourceLoc& begLoc, const SourceRange& tyRange, const SourceLoc& endLoc) :
	NamedDecl(DeclKind::VarDecl, id, begLoc, endLoc), type_(type), typeRange_(tyRange), init_(init)
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
	return init_;
}

const Expr* VarDecl::getInitExpr() const
{
	return init_;
}

bool VarDecl::hasInitExpr() const
{
	return (bool)init_;
}

void VarDecl::setType(const QualType &ty)
{
	type_ = ty;
}

void VarDecl::setInitExpr(Expr* expr)
{
	init_ = expr;
}

// ASTUnit
UnitDecl::UnitDecl(IdentifierInfo * id,FileID inFile)
	: NamedDecl(DeclKind::UnitDecl,id,SourceLoc(),SourceLoc()), file_(inFile)
{
	declsAreValid_ = true;
}

void UnitDecl::addDecl(Decl* decl)
{
	// Update locs
	if (!isBegLocSet())
		setBegLoc(decl->getBegLoc());

	setEndLoc(decl->getEndLoc());

	// Check the decl that we're adding in.
	if (!decl->isValid())
		declsAreValid_ = false;

	decls_.push_back(decl);
}

Decl* UnitDecl::getDecl(std::size_t idx)
{
	assert(idx < decls_.size() && "out-of-range");
	return decls_[idx];
}

const Decl* UnitDecl::getDecl(std::size_t idx) const
{
	assert(idx < decls_.size() && "out-of-range");
	return decls_[idx];
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