////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTExpr.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "ASTExpr.hpp"
#include "IVisitor.hpp"
#include "Moonshot/Common/Types/FVTypeTraits.hpp"

#include <iostream> // std::cout for debug purposes
#include <sstream> // std::stringstream


using namespace Moonshot;

//IASTexpr
FoxType IASTExpr::getResultType() const
{
	return resultType_;
}

void IASTExpr::setResultType(const FoxType & ft)
{
	resultType_ = ft;
}

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

bool ASTBinaryExpr::isComplete() const
{
	return left_ && right_ && (op_ != binaryOperator::DEFAULT);
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
ASTCastExpr::ASTCastExpr(const FoxType& castGoal, std::unique_ptr<IASTExpr> nc)
{
	setCastGoal(castGoal);
	setChild(std::move(nc));
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

IASTExpr * ASTCastExpr::getChild()
{
	return child_.get();
}

void ASTCastExpr::setChild(std::unique_ptr<IASTExpr> nc)
{
	child_ = std::move(nc);
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
std::string ASTFunctionCallExpr::getFunctionName() const
{
	return funcname_;
}

ExprList * ASTFunctionCallExpr::getExprList()
{
	return args_.get();
}

void ASTFunctionCallExpr::setExprList(std::unique_ptr<ExprList> elist)
{
	args_ = std::move(elist);
}

void ASTFunctionCallExpr::setFunctionName(const std::string& fnname)
{
	funcname_ = fnname;
}

void ASTFunctionCallExpr::accept(IVisitor & vis)
{
	vis.visit(*this);
}

// MemberRefExpr
ASTMemberAccessExpr::ASTMemberAccessExpr(std::unique_ptr<IASTExpr> base, std::unique_ptr<IASTDeclRef> memb)
{
	base_ = std::move(base);
	member_ = std::move(memb);
}

void ASTMemberAccessExpr::accept(IVisitor & vis)
{
	vis.visit(*this);
}

IASTExpr * ASTMemberAccessExpr::getBase()
{
	return base_.get();
}

IASTDeclRef* ASTMemberAccessExpr::getMemberDeclRef() const
{
	return member_.get();
}

void ASTMemberAccessExpr::setBase(std::unique_ptr<IASTExpr> expr)
{
	base_ = std::move(expr);
}

void ASTMemberAccessExpr::setMemberDeclRef(std::unique_ptr<IASTDeclRef> memb)
{
	member_ = std::move(memb);
}

ASTArrayAccess::ASTArrayAccess(std::unique_ptr<IASTExpr> expr, std::unique_ptr<IASTExpr> idxexpr) :
	base_(std::move(expr)), accessIdxExpr_(std::move(idxexpr))
{
	
}

void ASTArrayAccess::accept(IVisitor & vis)
{
	vis.visit(*this);
}

void ASTArrayAccess::setBase(std::unique_ptr<IASTExpr> expr)
{
	base_ = std::move(expr);
}

void ASTArrayAccess::setAccessIndexExpr(std::unique_ptr<IASTExpr> expr)
{
	accessIdxExpr_ = std::move(expr);
}

IASTExpr* ASTArrayAccess::getBase()
{
	return base_.get();
}

IASTExpr* ASTArrayAccess::getAccessIndexExpr()
{
	return accessIdxExpr_.get();
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

void ExprList::iterate(std::function<void(IASTExpr*)> fn)
{
	for (const auto& elem : exprs_)
		fn(elem.get());
}

