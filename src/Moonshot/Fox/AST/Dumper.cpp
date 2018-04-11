////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Dumper.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Dumper.hpp"
// Exception
#include "Moonshot/Fox/Basic/Exceptions.hpp"
// type
#include "Moonshot/Common/Types/FoxValueUtils.hpp"
// Include nodes
#include "Moonshot/Fox/AST/ASTExpr.hpp"
#include "Moonshot/Fox/AST/ASTDecl.hpp"
#include "Moonshot/Fox/AST/ASTStmt.hpp"
// utils
#include "Moonshot/Fox/Basic/Utils.hpp"

using namespace Moonshot;

Dumper::Dumper(std::ostream & outstream, const unsigned char& offsettabs) : out_(outstream), offsetTabs_(offsettabs)
{

}

void Dumper::dumpUnit(ASTUnit & unit)
{
	out_ << "ASTUnit containing " << unit.getDeclCount() << " declaration.\n";
	curindent_++;
	for (auto it = unit.decls_beg(); it != unit.decls_end(); it++)
		(*it)->accept(*this);
	curindent_--;
}

void Dumper::visit(ASTBinaryExpr & node)
{
	std::string op = Util::getFromDict(Dicts::kBinopToStr_dict, node.getOp());
	if (op.size() == 0)
		op = std::to_string(Util::enumAsInt(node.getOp()));

	out_ << getIndent() << "BinaryExpression : Operator " << op;
	// print planned result type if there's one
	if (node.getResultType() != 0 && node.getResultType() != TypeIndex::InvalidIndex)
		out_ << ", Return type : " << node.getResultType().getTypeName();
	// newline
	out_ << "\n";

	if (!node.isComplete())
	{
		throw Exceptions::ast_malformation("BinaryExpression node did not have a left and right child.");
		return;
	}
	else
	{
		// PRINT LEFT CHILD
		curindent_++;
		out_ << getIndent() << "Left child:\n";
		curindent_++;
		node.getLHS()->accept(*this);
		curindent_ -= 2;
		// PRINT RIGHT CHILD
		curindent_++;
		out_ << getIndent() << "Right child:\n";
		curindent_++;
		node.getRHS()->accept(*this);
		curindent_ -= 2;
	}
	
}

void Dumper::visit(ASTUnaryExpr & node)
{
	std::string op = Util::getFromDict(Dicts::kUnaryOpToStr_dict, node.getOp());
	if (op.size() == 0)
		op = std::to_string(Util::enumAsInt(node.getOp()));

	out_ << getIndent() << "UnaryExpression : Operator " << op;

	if (node.getResultType() != 0 && node.getResultType() != TypeIndex::InvalidIndex)
		out_ << ", Return type : " << node.getResultType().getTypeName();

	out_ << "\n";

	curindent_++;
	out_ << getIndent() << "Child:\n";
	curindent_++;

	if (!node.getChild())
	{
		throw Exceptions::ast_malformation("UnaryExpression node did not have a child.");
		return;
	}

	node.getChild()->accept(*this);
	curindent_ -= 2;
}

void Dumper::visit(ASTCastExpr & node)
{
	out_ << getIndent() << "CastExpression : Cast Goal:" << node.getCastGoal().getTypeName() << "\n";
	curindent_++;
	out_ << getIndent() << "Child:\n";
	curindent_++;

	if (!node.getChild())
	{
		throw Exceptions::ast_malformation("CastExpression node did not have a child.");
		return;
	}

	node.getChild()->accept(*this);
	curindent_ -= 2;
}
void Dumper::visit(ASTLiteralExpr & node)
{
	out_ << getIndent() << "Literal: " << FValUtils::dumpFVal(node.getVal()) << '\n';
}

void Dumper::visit(ASTVarDecl & node)
{
	out_ << getIndent() << "VarDeclStmt :" << node.getVarAttr().dump() << std::endl;
	if (node.hasInitExpr())
	{
		curindent_ += 1;
		out_ << getIndent() << "InitExpr\n";
		curindent_ += 1;
		node.getInitExpr()->accept(*this);
		curindent_ -= 2;
	}
}

void Dumper::visit(ASTMemberAccessExpr & node)
{
	out_ << getIndent() << "MemberOf Expr:\n";
	curindent_++;
	out_ << getIndent() << "Base:\n";
	curindent_++;
	node.getBase()->accept(*this);
	curindent_--;
	out_ << getIndent() << "Member:\n";
	curindent_++;
	node.getMemberDeclRef()->accept(*this);
	curindent_ -= 2;
}

void Dumper::visit(ASTArrayAccess & node)
{
	out_ << getIndent() << "ArrayAccess Expr:\n";
	curindent_++;
	out_ << getIndent() << "Base:\n";
	curindent_++;
	node.getBase()->accept(*this);
	curindent_--;
	out_ << getIndent() << "Index expression:\n";
	curindent_++;
	node.getAccessIndexExpr()->accept(*this);
	curindent_ -= 2;
}

void Dumper::visit(ASTDeclRefExpr & node)
{
	out_ << getIndent() << "DeclRef: name: " << node.getDeclnameStr() << std::endl;
}

void Dumper::visit(ASTFunctionCallExpr & node)
{
	out_ << getIndent() << "Function Call\n";
	curindent_++;
	out_ << getIndent() << "Function name :" << node.getFunctionName() << "\n";

	if (node.getExprList()->size())
	{
		out_ << getIndent() << "Args:\n";
		curindent_++;

		std::size_t count = 0;
		node.getExprList()->iterate([&](auto arg) {
			out_ << getIndent() << "Arg" << count << '\n';

			curindent_++;
			arg->accept(*this);
			curindent_--;

			count++;
		});

		curindent_--;
	}
	curindent_--;
}

void Dumper::visit(ASTNullStmt&)
{
	out_ << getIndent() << "Null Statement\n";
}

void Dumper::visit(ASTFunctionDecl & node)
{
	out_ << getIndent() << "Function Declaration : name:" << node.getName() << " return type:" << node.getReturnType().getTypeName() << "\n";
	curindent_ += 2;
	std::size_t counter = 0;
	node.iterateArgs([&](auto argdecl){
		out_ << getIndent() << "Arg" << counter << ":" << argdecl.dump() << std::endl;
		counter += 1;
	});
	curindent_ -= 1;
	out_ << getIndent() << "Body:" << std::endl;
	curindent_ += 1;
	node.getBody()->accept(*this);
	curindent_ -= 2;
}

void Dumper::visit(ASTReturnStmt & node)
{
	out_ << getIndent() << "Return statement\n";
	if (node.hasExpr())
	{
		curindent_ += 1;
		node.getExpr()->accept(*this);
		curindent_ -= 1;
	}
}

void Dumper::visit(ASTCompoundStmt & node)
{
	out_ << getIndent() << "Compound Statement (Contains " << node.size() << " statements)\n";

	curindent_ += 1;

	node.iterateStmts([&](auto stmt) {
		stmt->accept(*this);
	});

	curindent_ -= 1;
}

void Dumper::visit(ASTCondStmt & node)
{
	out_ << getIndent() << "Condition\n";
	curindent_++;
	// if
	out_ << getIndent() << "Expression (Condition):\n";
	curindent_++;
	node.getCond()->accept(*this);
	curindent_--;
	out_ << getIndent() << "Body:\n";
	curindent_++;
	node.getThen()->accept(*this);
	curindent_--;
	// has else?
	if (node.getElse())
	{
		out_ << getIndent() << "Else:\n";
		curindent_++;
		node.getElse()->accept(*this);
		curindent_--;
	}
	curindent_--;
}

void Dumper::visit(ASTWhileStmt & node)
{
	out_ << getIndent() << "While Loop\n";

	curindent_++;
	out_ << getIndent() << "Expression:\n";

	curindent_++;
	node.getCond()->accept(*this);
	curindent_--;

	out_ << getIndent() << "Body:\n";
	
	curindent_++;
	node.getBody()->accept(*this);
	curindent_--;

	curindent_--;
}

std::string Dumper::getIndent() const
{
	std::string i = getOffsetTabs();
	for (unsigned char k(0); k < curindent_; k++)
		i += "    ";
	if (curindent_ > 1)
		i += u8"┗";
	return i;
}

std::string Dumper::getOffsetTabs() const
{
	std::string i;
	for (unsigned char k(0); k < offsetTabs_; k++)
		i += '\t';
	return i;
}


