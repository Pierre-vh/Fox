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

#include "IVisitor.hpp"

using namespace Moonshot;

// return stmt
ASTReturnStmt::ASTReturnStmt(std::unique_ptr<ASTExpr> rtr_expr)
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

ASTExpr * ASTReturnStmt::getExpr()
{
	return expr_.get();
}

void ASTReturnStmt::setExpr(std::unique_ptr<ASTExpr> e)
{
	expr_ = std::move(e);
}

// cond stmt
ASTCondStmt::ASTCondStmt(std::unique_ptr<ASTExpr> cond, std::unique_ptr<ASTStmt> then, std::unique_ptr<ASTStmt> elsestmt)
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

ASTExpr * ASTCondStmt::getCond()
{
	return cond_.get();
}

ASTStmt * ASTCondStmt::getThen()
{
	return then_.get();
}

ASTStmt * ASTCondStmt::getElse()
{
	return else_.get();
}

void ASTCondStmt::setCond(std::unique_ptr<ASTExpr> expr)
{
	cond_ = std::move(expr);
}

void ASTCondStmt::setThen(std::unique_ptr<ASTStmt> then)
{
	then_ = std::move(then);
}

void ASTCondStmt::setElse(std::unique_ptr<ASTStmt> elsestmt)
{
	else_ = std::move(elsestmt);
}

// Compound stmt
void ASTCompoundStmt::accept(IVisitor & vis)
{
	vis.visit(*this);
}

ASTStmt * ASTCompoundStmt::getStmt(const std::size_t & ind)
{
	if (ind > stmts_.size())
		throw std::out_of_range("out of range");

	return stmts_[ind].get();
}

ASTStmt * ASTCompoundStmt::getBack()
{
	return stmts_.back().get();
}

void ASTCompoundStmt::addStmt(std::unique_ptr<ASTStmt> stmt)
{
	stmts_.emplace_back(std::move(stmt));
}

bool ASTCompoundStmt::isEmpty() const
{
	return !(stmts_.size());
}

std::size_t ASTCompoundStmt::size() const
{
	return stmts_.size();
}

ASTCompoundStmt::stmtvec::iterator ASTCompoundStmt::stmtList_beg()
{
	return stmts_.begin();
}

ASTCompoundStmt::stmtvec::iterator ASTCompoundStmt::stmtList_end()
{
	return stmts_.end();
}

void ASTCompoundStmt::iterateStmts(std::function<void(ASTStmt*)> fn)
{
	for (const auto& elem : stmts_)
		fn(elem.get());
}

// While stmt
ASTWhileStmt::ASTWhileStmt(std::unique_ptr<ASTExpr> cond, std::unique_ptr<ASTStmt> body)
{
	setCond(std::move(cond));
	setBody(std::move(body));
}

void ASTWhileStmt::accept(IVisitor & vis)
{
	vis.visit(*this);
}

ASTExpr * ASTWhileStmt::getCond()
{
	return cond_.get();
}

ASTStmt * ASTWhileStmt::getBody()
{
	return body_.get();
}

void ASTWhileStmt::setCond(std::unique_ptr<ASTExpr> cond)
{
	cond_ = std::move(cond);
}

void ASTWhileStmt::setBody(std::unique_ptr<ASTStmt> body)
{
	body_ = std::move(body);
}
