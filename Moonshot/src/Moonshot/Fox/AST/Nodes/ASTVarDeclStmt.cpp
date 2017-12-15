#include "ASTVarDeclStmt.h"

using namespace Moonshot;

ASTVarDeclStmt::ASTVarDeclStmt(const var::varattr & attr, std::unique_ptr<ASTExpr>& iExpr)
{
	if (attr)
	{
		vattr_ = attr;
		if (iExpr)
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
	vis.visit(this);
}

FVal ASTVarDeclStmt::accept(IRTVisitor& vis)
{
	vis.visit(this);
	return FVal(); // Doesn't return a value, just return something empty.
}

// varattr

var::varattr::varattr()
{
}

var::varattr::varattr(const std::string & nm, const std::size_t & ty, const bool & isK) : name(nm), type(ty), isConst(isK)
{
	wasInit_ = true;
}

var::varattr::operator bool() const
{
	return (wasInit_ && (type != fval_void) && (type != invalid_index));
}
