#include "ASTVarDeclStmt.h"

using namespace Moonshot;
using namespace fv_util;

ASTVarDeclStmt::ASTVarDeclStmt(const var::varattr & attr, std::unique_ptr<ASTExpr>& iExpr)
{
	if (attr)
	{
		vattr_ = attr;
		if (iExpr)						// if iexpr is valid, move it to our attribute.
			initExpr_ = std::move(iExpr);
	}
	else
		E_CRITICAL("Supplied an empty var::varattr object to the constructor.");
}

ASTVarDeclStmt::~ASTVarDeclStmt()
{

}

void ASTVarDeclStmt::accept(IVisitor& vis)
{
	vis.visit(*this);
}

FVal ASTVarDeclStmt::accept(IRTVisitor& vis)
{
	vis.visit(*this);
	return FVal(); // Doesn't return a value, just return something empty.
}

