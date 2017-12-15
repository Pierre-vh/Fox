#include "ASTExpr.h"

using namespace Moonshot;

ASTExpr::ASTExpr()
{

}

ASTExpr::ASTExpr(const parse::optype & opt) : op_(opt)
{

}

ASTExpr::~ASTExpr()
{

}

void ASTExpr::makeChild(const parse::direction & d, std::unique_ptr<ASTExpr> &node)
{
	if (d == parse::direction::LEFT)
		left_ = std::move(node);
	else if (d == parse::direction::RIGHT)
		right_ = std::move(node);
}

void Moonshot::ASTExpr::makeChildOfDeepestNode(const parse::direction & d, std::unique_ptr<ASTExpr>& node)
{
	if (d == parse::direction::LEFT)
	{
		if (!left_)						// we don't have a left child
			this->makeChild(d, node);
		else // we do
			left_->makeChildOfDeepestNode(d, node);
	}
	else if (d == parse::direction::RIGHT)
	{
		if (!right_)						// we don't have a right child
			this->makeChild(d, node);
		else // we do
			right_->makeChildOfDeepestNode(d, node);
	}
}

ASTExpr* Moonshot::ASTExpr::getDeepestNode(const parse::direction & d)
{
	if (d == parse::direction::LEFT)
	{
		if (left_)
			return left_->getDeepestNode(d);
		else
			return this;
	}
	else if (d == parse::direction::RIGHT)
	{
		if (right_)
			return right_->getDeepestNode(d);
		else
			return this;
	}
	E_CRITICAL("Unknown direction for ASTExpr node in getDeepestNode function.");
	return nullptr;
}

bool ASTExpr::hasNode(const parse::direction & d) const
{
	if (((d == parse::LEFT) && left_) || ((d == parse::RIGHT) && right_))
		return true;
	return false;
}

std::unique_ptr<ASTExpr> ASTExpr::getSimple()
{
	if (left_ && !right_ && (op_ == parse::optype::PASS))		// If the right node is empty
	{
		auto ret = std::move(left_);
		return ret;
	}
	return std::unique_ptr<ASTExpr>(nullptr);
}

void ASTExpr::accept(IVisitor& vis)
{
	vis.visit(this);
}

FVal ASTExpr::accept(IRTVisitor& vis)
{
	return vis.visit(this);
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

ASTValue::ASTValue()
{

}

ASTValue::ASTValue(const token & t)
{
	try
	{
		if (t.val_type == lex::VAL_STRING)
			val_ = t.str;
		else if (t.val_type == lex::VAL_CHAR)
			val_ = (char)t.str[0];
		else if (t.val_type == lex::VAL_BOOL)
			val_ = std::get<bool>(t.vals);
		else if (t.val_type == lex::VAL_INTEGER)
			val_ = std::get<int>(t.vals);
		else if (t.val_type == lex::VAL_FLOAT)
			val_ = std::get<float>(t.vals);
	}
	catch (const std::bad_variant_access &err)
	{
		E_CRITICAL("Tried to access a value in a variant that did not exists. ");
			std::cerr << err.what() << std::endl;
	}
}

void ASTValue::accept(IVisitor& vis)
{
	vis.visit(this);
}
FVal ASTValue::accept(IRTVisitor& vis)
{
	return vis.visit(this);
}

ASTValue::~ASTValue()
{

}


