////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTStmt.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "ASTStmt.hpp"
#include "ASTExpr.hpp"

using namespace Moonshot;

// null stmt
void ASTNullStmt::accept(IVisitor& vis)
{ 
	vis.visit(*this);
}

// return stmt
ASTReturnStmt::ASTReturnStmt(std::unique_ptr<IASTExpr> rtr_expr)
{
	expr_ = std::move(rtr_expr);
}

void ASTReturnStmt::accept(IVisitor & vis)
{
	vis.visit(*this);
}

bool ASTReturnStmt::hasExpr() const
{
	return (bool)expr_;
}

IASTExpr * ASTReturnStmt::getExpr()
{
	return expr_.get();
}

void ASTReturnStmt::setExpr(std::unique_ptr<IASTExpr> e)
{
	expr_ = std::move(e);
}

// cond stmt
ASTCondStmt::ASTCondStmt(std::unique_ptr<IASTExpr> cond, std::unique_ptr<IASTStmt> then, std::unique_ptr<IASTStmt> elsestmt)
{
	setCond(std::move(cond));
	setThen(std::move(then));
	setElse(std::move(elsestmt));
}

void ASTCondStmt::accept(IVisitor & vis)
{
	vis.visit(*this);
}

bool ASTCondStmt::isValid() const
{
	return cond_ && then_;
}

bool ASTCondStmt::hasElse() const
{
	return (bool)else_;
}

IASTExpr * ASTCondStmt::getCond()
{
	return cond_.get();
}

IASTStmt * ASTCondStmt::getThen()
{
	return then_.get();
}

IASTStmt * ASTCondStmt::getElse()
{
	return else_.get();
}

void ASTCondStmt::setCond(std::unique_ptr<IASTExpr> expr)
{
	cond_ = std::move(expr);
}

void ASTCondStmt::setThen(std::unique_ptr<IASTStmt> then)
{
	then_ = std::move(then);
}

void ASTCondStmt::setElse(std::unique_ptr<IASTStmt> elsestmt)
{
	else_ = std::move(elsestmt);
}

// Compound stmt
void ASTCompoundStmt::accept(IVisitor & vis)
{
	vis.visit(*this);
}

IASTStmt * ASTCompoundStmt::getStmt(const std::size_t & ind)
{
	if (ind > statements_.size())
		throw std::out_of_range("out of range");

	return statements_[ind].get();
}

IASTStmt * ASTCompoundStmt::getBack()
{
	return statements_.back().get();
}

void ASTCompoundStmt::addStmt(std::unique_ptr<IASTStmt> stmt)
{
	statements_.emplace_back(std::move(stmt));
}

bool ASTCompoundStmt::isEmpty() const
{
	return !(statements_.size());
}

std::size_t ASTCompoundStmt::size() const
{
	return statements_.size();
}

ASTCompoundStmt::stmtvec::iterator ASTCompoundStmt::stmtList_beg()
{
	return statements_.begin();
}

ASTCompoundStmt::stmtvec::iterator ASTCompoundStmt::stmtList_end()
{
	return statements_.end();
}

void ASTCompoundStmt::iterate(std::function<void(IASTStmt*)> fn)
{
	for (auto it = statements_.begin(); it != statements_.end(); it++)
		fn((*it).get());
}

// While stmt
ASTWhileStmt::ASTWhileStmt(std::unique_ptr<IASTExpr> cond, std::unique_ptr<IASTStmt> body)
{
	setCond(std::move(cond));
	setBody(std::move(body));
}

void ASTWhileStmt::accept(IVisitor & vis)
{
	vis.visit(*this);
}

IASTExpr * ASTWhileStmt::getCond()
{
	return cond_.get();
}

IASTStmt * ASTWhileStmt::getBody()
{
	return body_.get();
}

void ASTWhileStmt::setCond(std::unique_ptr<IASTExpr> cond)
{
	cond_ = std::move(cond);
}

void ASTWhileStmt::setBody(std::unique_ptr<IASTStmt> body)
{
	body_ = std::move(body);
}
