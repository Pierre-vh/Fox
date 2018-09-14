////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Decl.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Fox/AST/Expr.hpp"
#include "Fox/AST/Decl.hpp"
#include "Fox/AST/Stmt.hpp"
#include "Fox/Common/Source.hpp"
#include "Fox/AST/ASTContext.hpp"
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

void Decl::setRange(SourceRange range)
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

NamedDecl::NamedDecl(DeclKind kind, Identifier* id, SourceRange range):
	Decl(kind, range), identifier_(id)
{

}

Identifier* NamedDecl::getIdentifier()
{
	return identifier_;
}

const Identifier* NamedDecl::getIdentifier() const
{
	return identifier_;
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
// ValueDecl //
//-----------//

ValueDecl::ValueDecl(DeclKind kind, Identifier* id, Type* ty, bool isConst,
	SourceRange typeRange, SourceRange range):
	NamedDecl(kind, id, range), isConst_(isConst), tyRange_(typeRange),
	type_(ty)
{

}

Type* ValueDecl::getType()
{
	return type_;
}

const Type* ValueDecl::getType() const
{
	return type_;
}

void ValueDecl::setType(Type* ty)
{
	type_ = ty;
}

SourceRange ValueDecl::getTypeRange() const
{
	return tyRange_;
}

void ValueDecl::setTypeRange(SourceRange range)
{
	tyRange_ = range;
}

bool ValueDecl::isConstant() const
{
	return isConst_;
}

void ValueDecl::setIsConstant(bool k)
{
	isConst_ = k;
}

bool ValueDecl::isValid() const
{
	return NamedDecl::isValid() && type_ && tyRange_;
}

//-----------//
// ParamDecl //
//-----------//

ParamDecl::ParamDecl():
	ParamDecl(nullptr, nullptr, false, SourceRange(), SourceRange())
{

}

ParamDecl::ParamDecl(Identifier* id, Type* type, bool isConst, 
	SourceRange tyRange, SourceRange range):
	ValueDecl(DeclKind::ParamDecl, id, type, isConst, tyRange, range)
{

}

bool ParamDecl::isValid() const
{
	return ValueDecl::isValid();
}

//----------//
// FuncDecl //
//----------//

FuncDecl::FuncDecl():
	FuncDecl(nullptr, nullptr, nullptr, SourceRange(), SourceLoc())
{

}

FuncDecl::FuncDecl(Type* returnType, Identifier* fnId, CompoundStmt* body,
	SourceRange range, SourceLoc headerEndLoc):
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
	VarDecl(nullptr, nullptr, false, nullptr, SourceRange(), SourceRange())
{

}

VarDecl::VarDecl(Identifier* id, Type* type, bool isConst, Expr* init, 
	SourceRange tyRange, SourceRange range):
	ValueDecl(DeclKind::VarDecl, id, type, isConst, tyRange, range), init_(init)
{

}

bool VarDecl::isValid() const
{
	return ValueDecl::isValid();
}

Expr* VarDecl::getInitExpr() const
{
	return init_;
}

bool VarDecl::hasInitExpr() const
{
	return (bool)init_;
}

void VarDecl::setInitExpr(Expr* expr)
{
	init_ = expr;
}

//----------//
// UnitDecl //
//----------//

UnitDecl::UnitDecl(Identifier* id,FileID inFile)
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

const Decl* UnitDecl::getDecl(std::size_t idx) const
{
	assert(idx < decls_.size() && "out-of-range");
	return decls_[idx];
}

Decl* UnitDecl::getDecl(std::size_t idx)
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