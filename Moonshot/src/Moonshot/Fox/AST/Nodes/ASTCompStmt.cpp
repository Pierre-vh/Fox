#include "ASTCompStmt.h"

using namespace Moonshot;

ASTCompStmt::ASTCompStmt()
{
}


ASTCompStmt::~ASTCompStmt()
{
}

void ASTCompStmt::accept(IVisitor & vis)
{
	vis.visit(*this);
}

FVal ASTCompStmt::accept(IRTVisitor & vis)
{
	return vis.visit(*this);

}
