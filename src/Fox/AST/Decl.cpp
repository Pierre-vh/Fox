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

//------//
// Decl //
//------//

Decl::Decl(DeclKind kind, const SourceRange& range):
	kind_(kind), range_(range)
{

}

DeclKind Decl::getKind() const
{
	return kind_;
}

void Decl::setRange(const SourceRange& range)
{
	range_ = range;
}

SourceRange Decl::getRange() const
{
	return range_;
}

bool fox::Decl::isValid() const
{
	return range_.isValid();
}

void* Decl::operator new(std::size_t sz, ASTContext& ctxt, std::uint8_t align)
{
	return ctxt.getAllocator().allocate(sz, align);
}

//-----------//
// NamedDecl //
//-----------//

NamedDecl::NamedDecl(DeclKind kind, Identifier* id, const SourceRange& range):
	Decl(kind, range), identifier_(id)
{

}

Identifier* NamedDecl::getIdentifier() const
{
	return identifier_;;
}

void NamedDecl::setIdentifier(Identifier* nname)
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

//-----------//
// ParamDecl //
//-----------//

ParamDecl::ParamDecl() : ParamDecl(nullptr, QualType(), SourceRange(), SourceRange())
{

}

ParamDecl::ParamDecl(Identifier* id, const QualType& type, 
	const SourceRange& range, const SourceRange& tyRange):
	NamedDecl(DeclKind::ParamDecl, id, range), type_(type), tyRange_(tyRange)
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

//----------//
// FuncDecl //
//----------//

FuncDecl::FuncDecl():
	FuncDecl(nullptr, nullptr, nullptr, SourceRange(), SourceLoc())
{

}

FuncDecl::FuncDecl(Type* returnType, Identifier* fnId, CompoundStmt* body,
	const SourceRange& range, const SourceLoc& headerEndLoc):
	NamedDecl(DeclKind::FuncDecl, fnId, range), headEndLoc_(headerEndLoc), body_(body), returnType_(returnType)
{
	paramsAreValid_ = true;
}

void FuncDecl::setLocs(const SourceRange& range, const SourceLoc& headerEndLoc)
{
	setRange(range);
	setHeaderEndLoc(headerEndLoc);
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
	return SourceRange(getRange().getBegin(), headEndLoc_);
}

bool FuncDecl::isValid() const
{
	return NamedDecl::isValid() && body_ && returnType_ && headEndLoc_ && paramsAreValid_;
}

void FuncDecl::setReturnType(Type* ty)
{
	returnType_ = ty;
}

Type* FuncDecl::getReturnType() const
{
	return returnType_;
}

CompoundStmt* FuncDecl::getBody() const
{
	return body_;
}

void FuncDecl::setBody(CompoundStmt* body)
{
	body_ = body;
}

ParamDecl* FuncDecl::getParamDecl(std::size_t ind) const
{
	assert(ind < params_.size() && "out-of-range");
	return params_[ind];
}

void FuncDecl::addParam(ParamDecl* arg)
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

//---------//
// VarDecl //
//---------//

VarDecl::VarDecl():
	VarDecl(nullptr, QualType(), nullptr, SourceRange(), SourceRange())
{

}

VarDecl::VarDecl(Identifier* id, const QualType& type, Expr* init, 
	const SourceRange& range, const SourceRange& tyRange):
	NamedDecl(DeclKind::VarDecl, id, range), type_(type), typeRange_(tyRange), init_(init)
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

Expr* VarDecl::getInitExpr() const
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

//----------//
// UnitDecl //
//----------//

UnitDecl::UnitDecl(Identifier * id,FileID inFile)
	: NamedDecl(DeclKind::UnitDecl,id, SourceRange()), file_(inFile)
{
	declsAreValid_ = true;
}

void UnitDecl::addDecl(Decl* decl)
{
	// Check the decl
	if (!decl->isValid())
		declsAreValid_ = false;

	// Update locs
	SourceRange range;
	if (!getRange().isValid()) // (range not set yet)
	{
		assert((decls_.size() == 0) && "Range not set, but we already have decls?");
		range = decl->getRange();
	}
	else
	{
		assert((decls_.size() > 0) && "Range set, but we don't have decls?");
		range = SourceRange(
			getRange().getBegin(),
			decl->getRange().getEnd()
		);
	}
	assert(range && "Range is invalid");
	setRange(range);

	// Push it
	decls_.push_back(decl);
}

Decl* UnitDecl::getDecl(std::size_t idx) const
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