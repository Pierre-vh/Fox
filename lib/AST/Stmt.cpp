////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Stmt.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Fox/AST/Stmt.hpp"
#include "Fox/AST/Decl.hpp"
#include "Fox/AST/Expr.hpp"
#include "Fox/AST/ASTContext.hpp"
#include <cassert>

using namespace fox;

//------//
// Stmt //
//------//

Stmt::Stmt(StmtKind skind, SourceRange range):
	kind_(skind), range_(range)
{

}

StmtKind Stmt::getKind() const
{
	return kind_;
}

SourceRange Stmt::getRange() const
{
	return range_;
}

void* Stmt::operator new(std::size_t sz, ASTContext& ctxt, std::uint8_t align)
{
	return ctxt.getAllocator().allocate(sz, align);
}

void Stmt::setRange(SourceRange range)
{
	range_ = range;
}

//----------//
// NullStmt //
//----------//

NullStmt::NullStmt() : NullStmt(SourceLoc())
{

}

NullStmt::NullStmt(SourceLoc semiLoc):
	Stmt(StmtKind::NullStmt, SourceRange(semiLoc))
{

}

void NullStmt::setSemiLoc(SourceLoc semiLoc)
{
	setRange(SourceRange(semiLoc));
}

SourceLoc NullStmt::getSemiLoc() const
{
	return getRange().getBegin();
}

//------------//
// ReturnStmt //
//------------//

ReturnStmt::ReturnStmt():
	ReturnStmt(nullptr, SourceRange())
{

}

ReturnStmt::ReturnStmt(Expr* rtr_expr, SourceRange range):
	Stmt(StmtKind::ReturnStmt, range), expr_(rtr_expr)
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

const Expr* ReturnStmt::getExpr() const
{
	return expr_;
}

void ReturnStmt::setExpr(Expr* e)
{
	expr_ = e;
}

//---------------//
// ConditionStmt //
//---------------//

ConditionStmt::ConditionStmt() 
	: ConditionStmt(nullptr, ASTNode(), ASTNode(), SourceRange(), SourceLoc())
{

}

ConditionStmt::ConditionStmt(Expr* cond, ASTNode then, ASTNode elsenode,
	SourceRange range, SourceLoc ifHeaderEndLoc):
	Stmt(StmtKind::ConditionStmt, range), cond_(cond), then_(then), 
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

Expr* ConditionStmt::getCond()
{
	return cond_;
}

const Expr* ConditionStmt::getCond() const
{
	return cond_;
}

ASTNode ConditionStmt::getThen()
{
	return then_;
}

const ASTNode ConditionStmt::getThen() const
{
	return then_;
}

ASTNode ConditionStmt::getElse()
{
	return else_;
}

const ASTNode ConditionStmt::getElse() const
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

void ConditionStmt::setIfHeaderEndLoc(SourceLoc sloc)
{
	ifHeadEndLoc_ = sloc;
}

SourceRange ConditionStmt::getIfHeaderRange() const
{
	return SourceRange(getRange().getBegin(), ifHeadEndLoc_);
}

SourceLoc ConditionStmt::getIfHeaderEndLoc() const
{
	return ifHeadEndLoc_;
}

//--------------//
// CompoundStmt //
//--------------//

CompoundStmt::CompoundStmt() : CompoundStmt(SourceRange())
{

}

CompoundStmt::CompoundStmt(SourceRange range):
	Stmt(StmtKind::CompoundStmt, range)
{

}

ASTNode CompoundStmt::getNode(std::size_t ind)
{
	assert(ind < nodes_.size() && "out-of-range");
	return nodes_[ind];
}

const ASTNode CompoundStmt::getNode(std::size_t ind) const
{
	assert(ind < nodes_.size() && "out-of-range");
	return nodes_[ind];
}

CompoundStmt::NodeVecTy& CompoundStmt::getNodes()
{
	return nodes_;
}

void CompoundStmt::addNode(ASTNode node)
{
	nodes_.push_back(node);
}

void CompoundStmt::setNode(ASTNode node, std::size_t idx)
{
	assert((idx < nodes_.size()) && "Out of range");
	nodes_[idx] = node;
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

//-----------//
// WhileStmt //
//-----------//

WhileStmt::WhileStmt() : WhileStmt(nullptr, ASTNode(), SourceRange(), SourceLoc())
{

}

WhileStmt::WhileStmt(Expr* cond, ASTNode body, SourceRange range, SourceLoc headerEndLoc):
	Stmt(StmtKind::WhileStmt, range), headerEndLoc_(headerEndLoc), cond_(cond), body_(body)
{

}

Expr* WhileStmt::getCond()
{
	return cond_;
}

const Expr* WhileStmt::getCond() const
{
	return cond_;
}

ASTNode WhileStmt::getBody()
{
	return body_;
}

const ASTNode WhileStmt::getBody() const
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
	return SourceRange(getRange().getBegin(), headerEndLoc_);
}