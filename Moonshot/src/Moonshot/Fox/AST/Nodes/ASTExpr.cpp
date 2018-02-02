////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTExpr.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "ASTExpr.h"

using namespace Moonshot;

ASTLiteral::ASTLiteral(const Token & t)
{
	if (t.val_type == literalType::LIT_STRING)
		val_ = std::get<std::string>(t.vals);
	else if (t.val_type == literalType::LIT_CHAR)
		val_ = std::get<char>(t.vals);
	else if (t.val_type == literalType::LIT_BOOL)
		val_ = std::get<bool>(t.vals);
	else if (t.val_type == literalType::LIT_INTEGER)
		val_ = std::get<int>(t.vals);
	else if (t.val_type == literalType::LIT_FLOAT)
		val_ = std::get<float>(t.vals);
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

void ASTBinaryExpr::swapChildren()
{
	std::swap(left_, right_);
}

void ASTBinaryExpr::makeChild(const dir & d, std::unique_ptr<IASTExpr>& node)
{
	if (d == dir::LEFT)
		left_ = std::move(node);
	else if (d == dir::RIGHT)
		right_ = std::move(node);
}

void ASTBinaryExpr::makeChildOfDeepestNode(const dir & d, std::unique_ptr<IASTExpr>& node)
{
	if (d == dir::LEFT)
	{
		if (!left_)						// we don't have a left child
			left_ = std::move(node);
		else // we do
		{
			if(auto left_casted = dynamic_cast<ASTBinaryExpr*>(left_.get()))
				left_casted->makeChildOfDeepestNode(d, node);
		}
	}
	else if (d == dir::RIGHT)
	{
		if (!right_)						// we don't have a right child
			right_ = std::move(node);
		else // we do
		{
			if (auto right_casted = dynamic_cast<ASTBinaryExpr*>(right_.get()))
				right_casted->makeChildOfDeepestNode(d, node);
		}
	}
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
