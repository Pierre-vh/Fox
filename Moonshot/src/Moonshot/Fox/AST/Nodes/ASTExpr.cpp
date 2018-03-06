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

ASTLiteral::ASTLiteral(const FoxValue& fv)
{
	if (IndexUtils::isBasic(fv.index()))
		val_ = fv;
	else
		throw std::invalid_argument("ASTNodeLiteral constructor requires a basic type in the FoxValue");
}

void ASTLiteral::accept(IVisitor& vis)
{
	vis.visit(*this);
}

// VCalls
ASTIdentifier::ASTIdentifier(const std::string& vname) : identifier_str_(vname)
{

}

void ASTIdentifier::accept(IVisitor & vis)
{
	vis.visit(*this);
}

ASTBinaryExpr::ASTBinaryExpr(const binaryOperator & opt) : op_(opt)
{

}

void ASTBinaryExpr::accept(IVisitor & vis)
{
	vis.visit(*this);
}

std::unique_ptr<IASTExpr> ASTBinaryExpr::getSimple()
{
	if (left_ && !right_ && (op_ == binaryOperator::PASS))	 // If the right node is empty & op == pass
	{
		auto ret = std::move(left_);
		return ret;
	}
	return nullptr;
}

ASTUnaryExpr::ASTUnaryExpr(const unaryOperator & opt) : op_(opt)
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

void ASTCastExpr::setCastGoal(const FoxType& ncg)
{
	resultType_ = ncg;
}

FoxType ASTCastExpr::getCastGoal() const
{
	return resultType_;
}
