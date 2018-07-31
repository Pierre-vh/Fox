////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Stmt.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Stmt.hpp"
#include "Decl.hpp"
#include "Expr.hpp"
#include "ASTContext.hpp"
#include <cassert>

using namespace fox;

// Stmt
Stmt::Stmt(StmtKind skind, const SourceLoc& begLoc, const SourceLoc& endLoc) : kind_(skind), beg_(begLoc), end_(endLoc)
{

}

bool Stmt::isExpr() const
{
	return false;
}

StmtKind Stmt::getKind() const
{
	return kind_;
}

SourceRange Stmt::getRange() const
{
	return SourceRange(beg_,end_);
}

SourceLoc Stmt::getBegLoc() const
{
	return beg_;
}

SourceLoc Stmt::getEndLoc() const
{
	return end_;
}

bool Stmt::hasLocInfo() const
{
	return beg_ && end_;
}

void* Stmt::operator new(std::size_t sz, ASTContext& ctxt, std::uint8_t align)
{
	return ctxt.getAllocator().allocate(sz, align);
}

void Stmt::setBegLoc(const SourceLoc& loc)
{
	beg_ = loc;
}

void Stmt::setEndLoc(const SourceLoc& loc)
{
	end_ = loc;
}

bool Stmt::isBegLocSet() const
{
	return beg_.isValid();
}

bool Stmt::isEndLocSet() const
{
	return end_.isValid();
}

// ';' statement
NullStmt::NullStmt() : NullStmt(SourceLoc())
{

}

NullStmt::NullStmt(const SourceLoc& semiLoc) : Stmt(StmtKind::NullStmt,semiLoc,semiLoc)
{

}

void NullStmt::setSemiLoc(const SourceLoc& semiLoc)
{
	setBegLoc(semiLoc);
	setEndLoc(semiLoc);
}

SourceLoc NullStmt::getSemiLoc() const
{
	return getBegLoc();
}

// Return Statement
ReturnStmt::ReturnStmt() : ReturnStmt(nullptr,SourceLoc(),SourceLoc())
{
}

ReturnStmt::ReturnStmt(Expr* rtr_expr, const SourceLoc& begLoc, const SourceLoc& endLoc) 
	: Stmt(StmtKind::ReturnStmt,begLoc,endLoc), expr_(rtr_expr)
{

}

bool ReturnStmt::hasExpr() const
{
	return (bool)expr_;
}

Expr* ReturnStmt::getExpr()
{
	return expr_;
}

void ReturnStmt::setExpr(Expr* e)
{
	expr_ = e;
}

// Condition (if-then-else) statement
ConditionStmt::ConditionStmt() : ConditionStmt(nullptr,nullptr,nullptr,SourceLoc(),SourceLoc(),SourceLoc())
{

}

ConditionStmt::ConditionStmt(Expr* cond, Stmt* then, Stmt* elsestmt,
	const SourceLoc& begLoc, const SourceLoc& ifHeaderEndLoc, const SourceLoc& endLoc)
	: Stmt(StmtKind::ConditionStmt, begLoc, endLoc), cond_(cond), then_(then), 
	  else_(elsestmt), ifHeadEndLoc_(ifHeaderEndLoc)
{

}

bool ConditionStmt::isValid() const
{
	return cond_ && then_;
}

bool ConditionStmt::hasElse() const
{
	return (bool)else_;
}

Expr* ConditionStmt::getCond()
{
	return cond_;
}

Stmt* ConditionStmt::getThen()
{
	return then_;
}

Stmt* ConditionStmt::getElse()
{
	return else_;
}

const Expr* ConditionStmt::getCond() const
{
	return cond_;
}

const Stmt* ConditionStmt::getThen() const
{
	return then_;
}

const Stmt* ConditionStmt::getElse() const
{
	return else_;
}

void ConditionStmt::setCond(Expr* expr)
{
	cond_ = expr;
}

void ConditionStmt::setThen(Stmt* then)
{
	then_ = then;
}

void ConditionStmt::setElse(Stmt* stmt)
{
	else_ = stmt;
}

void ConditionStmt::setIfHeaderEndLoc(const SourceLoc& sloc)
{
	ifHeadEndLoc_ = sloc;
}

SourceRange ConditionStmt::getIfHeaderRange() const
{
	return SourceRange(getBegLoc(), ifHeadEndLoc_);
}

SourceLoc ConditionStmt::getIfHeaderEndLoc() const
{
	return ifHeadEndLoc_;
}

// Compound stmt
CompoundStmt::CompoundStmt() : CompoundStmt(SourceLoc(),SourceLoc())
{

}

CompoundStmt::CompoundStmt(const SourceLoc& begLoc, const SourceLoc& endLoc) : Stmt(StmtKind::CompoundStmt,begLoc,endLoc)
{

}

Stmt* CompoundStmt::getStmt(std::size_t ind)
{
	assert(ind < stmts_.size() && "out-of-range");
	return stmts_[ind];
}

const Stmt* CompoundStmt::getStmt(std::size_t ind) const
{
	assert(ind < stmts_.size() && "out-of-range");
	return stmts_[ind];
}

Stmt* CompoundStmt::getBack()
{
	return stmts_.back();
}

const Stmt* CompoundStmt::getBack() const
{
	return stmts_.back();
}

void CompoundStmt::addStmt(Stmt* stmt)
{
	stmts_.push_back(stmt);
}

bool CompoundStmt::isEmpty() const
{
	return !(stmts_.size());
}

std::size_t CompoundStmt::size() const
{
	return stmts_.size();
}

CompoundStmt::StmtVecIter CompoundStmt::stmts_beg()
{
	return stmts_.begin();
}

CompoundStmt::StmtVecIter CompoundStmt::stmts_end()
{
	return stmts_.end();
}

CompoundStmt::StmtVecConstIter CompoundStmt::stmts_beg() const
{
	return stmts_.begin();
}

CompoundStmt::StmtVecConstIter CompoundStmt::stmts_end() const
{
	return stmts_.end();
}

void CompoundStmt::setSourceLocs(const SourceLoc & begLoc, const SourceLoc & endLoc)
{
	setBegLoc(begLoc);
	setEndLoc(endLoc);
}

// While stmt
WhileStmt::WhileStmt() : WhileStmt(nullptr,nullptr,SourceLoc(),SourceLoc(),SourceLoc())
{

}

WhileStmt::WhileStmt(Expr* cond, Stmt* body, const SourceLoc& begLoc, const SourceLoc& headerEndLoc, const SourceLoc& endLoc) :
	Stmt(StmtKind::WhileStmt,begLoc,endLoc), headerEndLoc_(headerEndLoc), cond_(cond), body_(body)
{

}

Expr* WhileStmt::getCond()
{
	return cond_;
}

Stmt* WhileStmt::getBody()
{
	return body_;
}

const Expr* WhileStmt::getCond() const
{
	return cond_;
}

const Stmt* WhileStmt::getBody() const
{
	return body_;
}

void WhileStmt::setCond(Expr* cond)
{
	cond_ = cond;
}

void WhileStmt::setBody(Stmt* body)
{
	body_ = body;
}

SourceLoc WhileStmt::getHeaderEndLoc() const
{
	return headerEndLoc_;
}

SourceRange WhileStmt::getHeaderRange() const
{
	return SourceRange(getBegLoc(),headerEndLoc_);
}

// DeclStmt
DeclStmt::DeclStmt(Decl* decl) : Stmt(StmtKind::DeclStmt,SourceLoc(),SourceLoc())
{
	setDecl(decl);
}

bool DeclStmt::hasDecl() const
{
	return (bool)decl_;
}

Decl*  DeclStmt::getDecl()
{
	return decl_;
}

const Decl* DeclStmt::getDecl() const
{
	return decl_;
}

void DeclStmt::setDecl(Decl* decl)
{
	assert(decl && "The Decl cannot be null!");
	decl_ = decl;
	setBegLoc(decl_->getBegLoc());
	setEndLoc(decl_->getEndLoc());
}