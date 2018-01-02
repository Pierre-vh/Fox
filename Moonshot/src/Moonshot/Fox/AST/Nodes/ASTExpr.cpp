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

ASTExpr::ASTExpr(const operation & opt) : op_(opt)
{

}

ASTExpr::~ASTExpr()
{

}

void ASTExpr::makeChild(const dir & d, std::unique_ptr<ASTExpr> &node)
{
	if (d == dir::LEFT)
		left_ = std::move(node);
	else if (d == dir::RIGHT)
		right_ = std::move(node);
}

void ASTExpr::makeChildOfDeepestNode(const dir & d, std::unique_ptr<ASTExpr>& node)
{
	if (d == dir::LEFT)
	{
		if (!left_)						// we don't have a left child
			this->makeChild(d, node);
		else // we do
			left_->makeChildOfDeepestNode(d, node);
	}
	else if (d == dir::RIGHT)
	{
		if (!right_)						// we don't have a right child
			this->makeChild(d, node);
		else // we do
			right_->makeChildOfDeepestNode(d, node);
	}
}

bool ASTExpr::hasNode(const dir & d) const
{
	if (((d == dir::LEFT) && left_) || ((d == dir::RIGHT) && right_))
		return true;
	return false;
}

std::unique_ptr<ASTExpr> ASTExpr::getSimple()
{
	if (left_ && !right_ && (op_ == operation::PASS))		// If the right node is empty
	{
		auto ret = std::move(left_);
		return ret;
	}
	return std::unique_ptr<ASTExpr>(nullptr);
}

void ASTExpr::accept(IVisitor& vis)
{
	vis.visit(*this);
}

FVal ASTExpr::accept(IRTVisitor& vis)
{
	return vis.visit(*this);
}

void ASTExpr::setReturnType(const std::size_t &casttype)
{
	totype_ = casttype;
}

std::size_t ASTExpr::getToType() const
{
	return totype_;
}

void ASTExpr::swapChildren()
{
	std::swap(left_, right_);
}

ASTRawValue::ASTRawValue(const token & t)
{
	if (t.val_type == valueType::VAL_STRING)
		val_ = t.str;
	else if (t.val_type == valueType::VAL_CHAR)
		val_ = (char)t.str[0];
	else if (t.val_type == valueType::VAL_BOOL)
		val_ = std::get<bool>(t.vals);
	else if (t.val_type == valueType::VAL_INTEGER)
		val_ = std::get<int>(t.vals);
	else if (t.val_type == valueType::VAL_FLOAT)
		val_ = std::get<float>(t.vals);
}

void ASTRawValue::accept(IVisitor& vis)
{
	vis.visit(*this);
}
FVal ASTRawValue::accept(IRTVisitor& vis)
{
	return vis.visit(*this);
}

ASTRawValue::~ASTRawValue()
{

}
// VCalls
ASTVarCall::ASTVarCall(const std::string& vname) : varname_(vname)
{
}

ASTVarCall::~ASTVarCall()
{

}

void ASTVarCall::accept(IVisitor & vis)
{
	vis.visit(*this);
}

FVal ASTVarCall::accept(IRTVisitor & vis)
{
	return vis.visit(*this);
}
