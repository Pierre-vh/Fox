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

#include <cassert>

using namespace Moonshot;

// Stmt
Stmt::Stmt(const StmtKind & skind, const SourceLoc& begLoc, const SourceLoc& endLoc) : kind_(skind), beg_(begLoc), end_(endLoc)
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

ReturnStmt::ReturnStmt(std::unique_ptr<Expr> rtr_expr, const SourceLoc& begLoc, const SourceLoc& endLoc) : Stmt(StmtKind::ReturnStmt,begLoc,endLoc)
{
	expr_ = std::move(rtr_expr);
}

bool ReturnStmt::hasExpr() const
{
	return (bool)expr_;
}

Expr * ReturnStmt::getExpr()
{
	return expr_.get();
}

void ReturnStmt::setExpr(std::unique_ptr<Expr> e)
{
	expr_ = std::move(e);
}

// Condition (if-then-else) statement
ConditionStmt::ConditionStmt() : ConditionStmt(nullptr,nullptr,nullptr,SourceLoc(),SourceLoc(),SourceLoc())
{

}

ConditionStmt::ConditionStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> then, std::unique_ptr<Stmt> elsestmt,
	const SourceLoc& begLoc, const SourceLoc& ifHeaderEndLoc, const SourceLoc& endLoc)
	: Stmt(StmtKind::ConditionStmt, begLoc, endLoc)
{
	setCond(std::move(cond));
	setThen(std::move(then));
	setElse(std::move(elsestmt));
	setIfHeaderEndLoc(ifHeaderEndLoc);
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
	return cond_.get();
}

Stmt* ConditionStmt::getThen()
{
	return then_.get();
}

Stmt* ConditionStmt::getElse()
{
	return else_.get();
}

const Expr* ConditionStmt::getCond() const
{
	return cond_.get();
}

const Stmt* ConditionStmt::getThen() const
{
	return then_.get();
}

const Stmt* ConditionStmt::getElse() const
{
	return else_.get();
}

void ConditionStmt::setCond(std::unique_ptr<Expr> expr)
{
	cond_ = std::move(expr);
}

void ConditionStmt::setThen(std::unique_ptr<Stmt> then)
{
	then_ = std::move(then);
}

void ConditionStmt::setElse(std::unique_ptr<Stmt> elsestmt)
{
	else_ = std::move(elsestmt);
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

Stmt* CompoundStmt::getStmt(const std::size_t& ind)
{
	if (ind > stmts_.size())
		throw std::out_of_range("out of range");

	return stmts_[ind].get();
}

const Stmt* CompoundStmt::getStmt(const std::size_t& ind) const
{
	if (ind > stmts_.size())
		throw std::out_of_range("out of range");

	return stmts_[ind].get();
}

Stmt* CompoundStmt::getBack()
{
	return stmts_.back().get();
}

const Stmt* CompoundStmt::getBack() const
{
	return stmts_.back().get();
}

void CompoundStmt::addStmt(std::unique_ptr<Stmt> stmt)
{
	stmts_.emplace_back(std::move(stmt));
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

WhileStmt::WhileStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> body, const SourceLoc& begLoc, const SourceLoc& headerEndLoc, const SourceLoc& endLoc) :
	Stmt(StmtKind::WhileStmt,begLoc,endLoc), headerEndLoc_(headerEndLoc)
{
	setCond(std::move(cond));
	setBody(std::move(body));
}

Expr* WhileStmt::getCond()
{
	return cond_.get();
}

Stmt* WhileStmt::getBody()
{
	return body_.get();
}

const Expr* WhileStmt::getCond() const
{
	return cond_.get();
}

const Stmt* WhileStmt::getBody() const
{
	return body_.get();
}

void WhileStmt::setCond(std::unique_ptr<Expr> cond)
{
	cond_ = std::move(cond);
}

void WhileStmt::setBody(std::unique_ptr<Stmt> body)
{
	body_ = std::move(body);
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
DeclStmt::DeclStmt(std::unique_ptr<Decl> decl) : Stmt(StmtKind::DeclStmt,SourceLoc(),SourceLoc())
{
	setDecl(std::move(decl));
}

bool DeclStmt::hasDecl() const
{
	return (bool)decl_;
}

Decl*  DeclStmt::getDecl()
{
	return decl_.get();
}

const Decl* DeclStmt::getDecl() const
{
	return decl_.get();
}

void DeclStmt::setDecl(std::unique_ptr<Decl> decl)
{
	assert(decl && "The Decl cannot be null!");
	decl_ = std::move(decl);
	setBegLoc(decl_->getBegLoc());
	setEndLoc(decl_->getEndLoc());
}