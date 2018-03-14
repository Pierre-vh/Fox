////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTExpr.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "ASTExpr.hpp"

#include "Moonshot/Common/Types/FVTypeTraits.hpp"

#include <iostream> // std::cout for debug purposes
#include <sstream> // std::stringstream


using namespace Moonshot;

// Literal
ASTLiteralExpr::ASTLiteralExpr(const FoxValue& fv)
{
	if (IndexUtils::isBasic(fv.index()))
		setVal(fv);
	else
		throw std::invalid_argument("ASTNodeLiteral constructor requires a basic type in the FoxValue");
}

void ASTLiteralExpr::accept(IVisitor& vis)
{
	vis.visit(*this);
}

FoxValue ASTLiteralExpr::getVal() const
{
	return val_;
}

void ASTLiteralExpr::setVal(const FoxValue & nval)
{
	val_ = nval;
}


// BinaryExpr
ASTBinaryExpr::ASTBinaryExpr(const binaryOperator & opt, std::unique_ptr<IASTExpr> lhs, std::unique_ptr<IASTExpr> rhs):
	op_(opt)
{
	setLHS(std::move(lhs));
	setRHS(std::move(rhs));
}

void ASTBinaryExpr::accept(IVisitor & vis)
{
	vis.visit(*this);
}

std::unique_ptr<IASTExpr> ASTBinaryExpr::getSimple()
{
	if (left_ && !right_ && (op_ == binaryOperator::DEFAULT))	 // If the right node is empty & op == pass
	{
		auto ret = std::move(left_);
		return ret;
	}
	return nullptr;
}

IASTExpr * ASTBinaryExpr::getLHS()
{
	return left_.get();
}

IASTExpr * ASTBinaryExpr::getRHS()
{
	return right_.get();
}

void ASTBinaryExpr::setLHS(std::unique_ptr<IASTExpr> nlhs)
{
	left_ = std::move(nlhs);
}

void ASTBinaryExpr::setRHS(std::unique_ptr<IASTExpr> nrhs)
{
	right_ = std::move(nrhs);
}

binaryOperator ASTBinaryExpr::getOp() const
{
	return op_;
}

void ASTBinaryExpr::setOp(const binaryOperator & op)
{
	op_ = op;
}

// UnaryExpr
ASTUnaryExpr::ASTUnaryExpr(const unaryOperator & opt, std::unique_ptr<IASTExpr> node) : op_(opt)
{
	setChild(std::move(node));
}

void ASTUnaryExpr::accept(IVisitor & vis)
{
	vis.visit(*this);
}

IASTExpr * ASTUnaryExpr::getChild()
{
	return child_.get();
}

void ASTUnaryExpr::setChild(std::unique_ptr<IASTExpr> nchild)
{
	child_ = std::move(nchild);
}

unaryOperator ASTUnaryExpr::getOp() const
{
	return op_;
}

void ASTUnaryExpr::setOp(const unaryOperator & nop)
{
	op_ = nop;
}

// CastExpr
ASTCastExpr::ASTCastExpr(const FoxType& castGoal)
{
	setCastGoal(castGoal);
}

void ASTCastExpr::accept(IVisitor & vis)
{
	vis.visit(*this);
}

void ASTCastExpr::setCastGoal(const FoxType& ncg)
{
	resultType_ = ncg;
}

FoxType ASTCastExpr::getCastGoal() const
{
	return resultType_;
}

// DeclRefs
ASTDeclRefExpr::ASTDeclRefExpr(const std::string& vname) : declname_(vname)
{

}

void ASTDeclRefExpr::accept(IVisitor & vis)
{
	vis.visit(*this);
}

std::string ASTDeclRefExpr::getDeclnameStr() const
{
	return declname_;
}

void ASTDeclRefExpr::setDeclnameStr(const std::string & str)
{
	declname_ = str;
}

// declref
IASTDeclRef * ASTFunctionCallExpr::getDeclRefExpr()
{
	return declref_.get();
}

ExprList * ASTFunctionCallExpr::getExprList()
{
	return args_.get();
}

void ASTFunctionCallExpr::setExprList(std::unique_ptr<ExprList> elist)
{
	args_ = std::move(elist);
}

void ASTFunctionCallExpr::setDeclRef(std::unique_ptr<IASTDeclRef> dref)
{
	declref_ = std::move(dref);
}

void ASTFunctionCallExpr::accept(IVisitor & vis)
{
	vis.visit(*this);
}

// MemberRefExpr
ASTMemberOfExpr::ASTMemberOfExpr(std::unique_ptr<IASTExpr> base, const std::string & membname)
{
	base_ = std::move(base);
	memb_name_ = membname;
}

void ASTMemberOfExpr::accept(IVisitor & vis)
{
	vis.visit(*this);
}

IASTExpr * ASTMemberOfExpr::getBase()
{
	return base_.get();
}

std::string ASTMemberOfExpr::getMemberNameStr() const
{
	return memb_name_;
}

void ASTMemberOfExpr::setBase(std::unique_ptr<IASTExpr> expr)
{
	base_ = std::move(expr);
}

void ASTMemberOfExpr::setDeclname(const std::string& membname)
{
	memb_name_ = membname;
}

// Expr list
void ExprList::addExpr(std::unique_ptr<IASTExpr> expr)
{
	exprs_.emplace_back(std::move(expr));
}

const IASTExpr * ExprList::getExpr(const std::size_t & ind)
{
	if (ind > size())
		throw std::out_of_range("Tried to access an out of bounds location in an expression list.");

	return exprs_[ind].get();
}

bool ExprList::isEmpty() const
{
	return !exprs_.size();
}

std::size_t ExprList::size() const
{
	return exprs_.size();
}

ExprList::expr_iter ExprList::exprList_beg()
{
	return exprs_.begin();
}

ExprList::expr_iter ExprList::exprList_end()
{
	return exprs_.end();
}
