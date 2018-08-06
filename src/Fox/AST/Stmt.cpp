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
ConditionStmt::ConditionStmt() 
	: ConditionStmt(nullptr, ASTNode(), ASTNode(), SourceLoc(), SourceLoc(), SourceLoc())
{

}

ConditionStmt::ConditionStmt(Expr* cond, ASTNode then, ASTNode elsenode,
	const SourceLoc& begLoc, const SourceLoc& ifHeaderEndLoc, const SourceLoc& endLoc)
	: Stmt(StmtKind::ConditionStmt, begLoc, endLoc), cond_(cond), then_(then), 
	  else_(elsenode), ifHeadEndLoc_(ifHeaderEndLoc)
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

Expr* ConditionStmt::getCond() const
{
	return cond_;
}

ASTNode ConditionStmt::getThen() const
{
	return then_;
}

ASTNode ConditionStmt::getElse() const
{
	return else_;
}

void ConditionStmt::setCond(Expr* expr)
{
	cond_ = expr;
}

void ConditionStmt::setThen(ASTNode node)
{
	then_ = node;
}

void ConditionStmt::setElse(ASTNode node)
{
	else_ = node;
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

ASTNode CompoundStmt::getNode(std::size_t ind) const
{
	assert(ind < nodes_.size() && "out-of-range");
	return nodes_[ind];
}

void CompoundStmt::addNode(ASTNode node)
{
	nodes_.push_back(node);
}

bool CompoundStmt::isEmpty() const
{
	return !(nodes_.size());
}

std::size_t CompoundStmt::size() const
{
	return nodes_.size();
}

CompoundStmt::NodeVecTy::iterator CompoundStmt::nodes_begin()
{
	return nodes_.begin();
}

CompoundStmt::NodeVecTy::iterator CompoundStmt::nodes_end()
{
	return nodes_.end();
}

CompoundStmt::NodeVecTy::const_iterator CompoundStmt::nodes_begin() const
{
	return nodes_.begin();
}

CompoundStmt::NodeVecTy::const_iterator CompoundStmt::nodes_end() const
{
	return nodes_.end();
}

void CompoundStmt::setSourceLocs(const SourceLoc & begLoc, const SourceLoc & endLoc)
{
	setBegLoc(begLoc);
	setEndLoc(endLoc);
}

// While stmt
WhileStmt::WhileStmt() : WhileStmt(nullptr, ASTNode(), SourceLoc(), SourceLoc(), SourceLoc())
{

}

WhileStmt::WhileStmt(Expr* cond, ASTNode body, const SourceLoc& begLoc, const SourceLoc& headerEndLoc, const SourceLoc& endLoc) :
	Stmt(StmtKind::WhileStmt,begLoc,endLoc), headerEndLoc_(headerEndLoc), cond_(cond), body_(body)
{

}

Expr* WhileStmt::getCond() const
{
	return cond_;
}

ASTNode WhileStmt::getBody() const
{
	return body_;
}

void WhileStmt::setCond(Expr* cond)
{
	cond_ = cond;
}

void WhileStmt::setBody(ASTNode body)
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