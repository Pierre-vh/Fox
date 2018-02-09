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

ASTLiteral::ASTLiteral(const FVal& fv)
{
	if (fv_util::isBasic(fv.index()))
		val_ = fv;
	else
		throw std::invalid_argument("ASTNodeLiteral constructor requires a basic type in the FVal");
}

void ASTLiteral::accept(IVisitor& vis)
{
	vis.visit(*this);
}

// VCalls
ASTVarCall::ASTVarCall(const std::string& vname) : varname_(vname)
{

}

void ASTVarCall::accept(IVisitor & vis)
{
	vis.visit(*this);
}

ASTBinaryExpr::ASTBinaryExpr(const binaryOperation & opt) : op_(opt)
{

}

void ASTBinaryExpr::accept(IVisitor & vis)
{
	vis.visit(*this);
}

std::unique_ptr<IASTExpr> ASTBinaryExpr::getSimple()
{
	if (left_ && !right_ && (op_ == binaryOperation::PASS))	 // If the right node is empty & op == pass
	{
		auto ret = std::move(left_);
		return ret;
	}
	return nullptr;
}

void ASTBinaryExpr::setChild(const dir & d, std::unique_ptr<IASTExpr>& node)
{
	if (d == dir::LEFT)
		left_ = std::move(node);
	else if (d == dir::RIGHT)
		right_ = std::move(node);
}

void ASTBinaryExpr::makeChildOfDeepestNode(const dir & d, std::unique_ptr<IASTExpr>& node)
{
	ASTBinaryExpr* cur = this;
	if (d == dir::LEFT)
	{
		while (cur->left_)
		{
			if (auto isLeftBinop = dynamic_cast<ASTBinaryExpr*>(cur->left_.get()))
				cur = isLeftBinop;
		}
	}
	else if (d == dir::RIGHT)
	{
		while (cur->right_)
		{
			if (auto isRightBinop = dynamic_cast<ASTBinaryExpr*>(cur->right_.get()))
				cur = isRightBinop;
		}
	}
	cur->setChild(d, node);
}

ASTUnaryExpr::ASTUnaryExpr(const unaryOperation & opt) : op_(opt)
{

}

void ASTUnaryExpr::accept(IVisitor & vis)
{
	vis.visit(*this);
}

ASTCastExpr::ASTCastExpr(std::size_t castGoal)
{
	setCastGoal(castGoal);
}

void ASTCastExpr::accept(IVisitor & vis)
{
	vis.visit(*this);
}

void ASTCastExpr::setCastGoal(const std::size_t& ncg)
{
	resultType_ = ncg;
}

std::size_t ASTCastExpr::getCastGoal() const
{
	return resultType_;
}
