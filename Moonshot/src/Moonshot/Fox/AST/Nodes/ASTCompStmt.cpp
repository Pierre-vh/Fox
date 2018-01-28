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