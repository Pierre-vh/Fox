////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTDumper.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "ASTDumper.hpp"
#include "IdentifierTable.hpp"

#include <string>
#include <exception>
#include <sstream>

#define INDENT "  "
#define OFFSET_INDENT "\t"

using namespace Moonshot;


ASTDumper::ASTDumper(std::ostream & out, const uint8_t & offsettabs) : out_(out), offsetTabs_(offsettabs)
{
	recalculateOffset();
}

void ASTDumper::visitBinaryExpr(BinaryExpr * node)
{
	getOut() << getBasicStmtInfo(node) << '\n';

	// Print LHS 
	getOut(1) << "[LHS]\n";
	indent(2);
		visit(node->getLHS());
	dedent(2);

	// Print RHS
	getOut(1) << "[RHS]\n";
	indent(2);
		visit(node->getRHS());
	dedent(2);
}

void ASTDumper::visitCastExpr(CastExpr * node)
{
	getOut() << getBasicStmtInfo(node) << " -> to <" << node->getCastGoal()->getString() << ">\n";
	indent();
		visit(node->getChild());
	dedent();
}

void ASTDumper::visitUnaryExpr(UnaryExpr * node)
{
	getOut() << getBasicStmtInfo(node) << " -> Operator " << Operators::toString(node->getOp()) << '\n';
	indent();
		visit(node->getChild());
	dedent();
}

void ASTDumper::visitArrayAccessExpr(ArrayAccessExpr * node)
{
	getOut() << getBasicStmtInfo(node) << '\n';

	// Print Base 
	getOut(1) << "[Base]\n";
	indent(2);
		visit(node->getBase());
	dedent(2);

	// Print IdxExpr
	getOut(1) << "[Index]\n";
	indent(2);
		visit(node->getAccessIndexExpr());
	dedent(2);
}

void ASTDumper::visitMemberOfExpr(MemberOfExpr * node)
{
	getOut() << getBasicStmtInfo(node) << " -> ." << node->getMemberName()->getStr() << '\n';
	indent();
		visit(node->getBase());
	dedent();
}

void ASTDumper::visitDeclRefExpr(DeclRefExpr * node)
{
	getOut() << getBasicStmtInfo(node) << " -> \"" << node->getDeclIdentifier()->getStr() << "\"\n";
}

void ASTDumper::visitFunctionCallExpr(FunctionCallExpr * node)
{
	getOut() << getBasicStmtInfo(node) << '\n';

	// Print Base 
	getOut(1) << "[Callee]\n";
	indent(2);
		visit(node->getCallee());
	dedent(2);

	// Print Args if there are args
	if (node->getExprList() && (!node->getExprList()->isEmpty()))
	{
		getOut(1) << "[Args]\n";
		auto elist = node->getExprList();
		unsigned counter = 0;
		for (auto it = elist->begin(); it != elist->end(); it++, counter++)
		{
			getOut(2) << "[Arg " << counter << "]\n";
			indent(3);
				visit(*it);
			dedent(3);
		}
	}
}

void ASTDumper::visitNullExpr(NullExpr * node)
{
	getOut() << getBasicStmtInfo(node) << "\n";
}

void ASTDumper::visitCharLiteralExpr(CharLiteralExpr * node)
{
	getOut() << getBasicStmtInfo(node) << '\'' << node->getVal() << "'\n";
}

void ASTDumper::visitIntegerLiteralExpr(IntegerLiteralExpr * node)
{
	getOut() << getBasicStmtInfo(node) << node->getVal() << "\n";
}

void ASTDumper::visitFloatLiteralExpr(FloatLiteralExpr * node)
{
	getOut() << getBasicStmtInfo(node) << node->getVal() << "\n";
}

void ASTDumper::visitBooleanLiteralExpr(BoolLiteralExpr * node)
{
	getOut() << getBasicStmtInfo(node) << (node->getVal() ? "true" : "false") << "\n";
}

void ASTDumper::visitStringLiteralExpr(StringLiteralExpr * node)
{
	getOut() << getBasicStmtInfo(node) << '"' << node->getVal() << "\"\n";
}

void ASTDumper::visitArrayLiteralExpr(ArrayLiteralExpr * node)
{
	std::size_t elemcount = 0;
	if (node->hasExprList())
		elemcount = node->getExprList()->size();

	getOut() << getBasicStmtInfo(node) << "(" << elemcount << " elements)\n";

	if (node->hasExprList())
	{
		ExprList* elist = node->getExprList();
		uint16_t counter = 0;
		for (auto it = elist->begin(); it != elist->end(); it++)
		{
			getOut(1) << "[" << counter << "]\n";
			indent(2);
				visit(*it);
			dedent(2);
			counter++;
		}
	}
}

void ASTDumper::visitCompoundStmt(CompoundStmt * node)
{
	getOut() << getBasicStmtInfo(node) << " (" << node->size() << " statements)\n";
	indent();
	for (auto it = node->stmts_beg(); it != node->stmts_end(); it++)
		visit(*it);
	dedent();
}

void ASTDumper::visitConditionStmt(ConditionStmt * node)
{
	getOut() << getBasicStmtInfo(node) << "\n";
	// Visit cond
	getOut(1) << "[Cond Expr]\n";
	indent(2);
		visit(node->getCond());
	dedent(2);

	// Visit Then
	getOut(1) << "[Then]\n";
	indent(2);
		visit(node->getThen());
	dedent(2);

	// If there's a else, visit it
	if (node->hasElse())
	{
		getOut(1) << "[Else]\n";
		indent(2);
			visit(node->getElse());
		dedent(2);
	}
}

void ASTDumper::visitWhileStmt(WhileStmt * node)
{
	getOut() << getBasicStmtInfo(node) << "\n";
	// Visit cond
	getOut(1) << "[Cond Expr]\n";
	indent(2);
		visit(node->getCond());
	dedent(2);

	// Visit body
	getOut(1) << "[Body]\n";
	indent(2);
		visit(node->getBody());
	dedent(2);
}

void ASTDumper::visitDeclStmt(DeclStmt * node)
{
	getOut() << getBasicStmtInfo(node) << "\n";
	indent();
		visit(node->getDecl());
	dedent();
}

void ASTDumper::visitReturnStmt(ReturnStmt * node)
{
	getOut() << getBasicStmtInfo(node) << "\n";
	if (node->hasExpr())
	{
		getOut(1) << "[Expr]\n";
		indent(2);
			visit(node->getExpr());
		dedent(2);
	}
}

void ASTDumper::visitUnitDecl(UnitDecl * node)
{
	getOut() << getBasicDeclInfo(node) << " -> Id: \"" << node->getIdentifier()->getStr() << "\"\n";
	indent();
	for (auto it = node->decls_beg(); it != node->decls_end(); it++)
		visit(*it);
	dedent();
}

void ASTDumper::visitVarDecl(VarDecl * node)
{
	getOut() << getBasicDeclInfo(node) << " -> Id: \"" << node->getIdentifier()->getStr() << "\" Type: <" << node->getType().getString() << ">\n";
	if (node->hasInitExpr())
	{
		getOut(1) << "[Init]\n";
		indent(2);
			visit(node->getInitExpr());
		dedent(2);
	}
}

void ASTDumper::visitArgDecl(ArgDecl * node)
{
	getOut() << getBasicDeclInfo(node) << " -> Id: \"" << node->getIdentifier()->getStr() << "\" Type: <" << node->getType().getString() << ">\n";
}

void ASTDumper::visitFunctionDecl(FunctionDecl * node)
{
	getOut() << getBasicDeclInfo(node) << " -> Returns <" << node->getReturnType()->getString() << ">\n";
	if (node->argsSize())
	{
		getOut(1) << "[Args Decls]\n";
		unsigned counter = 0;
		for (auto it = node->args_begin(); it != node->args_end(); it++, counter++)
		{
			getOut(2) << "[Arg " << counter << "]\n";
			indent(3);
				visit(*it);
			dedent(3);
		}
	}
	// Visit the compound statement
	if (auto body = node->getBody())
	{
		indent();
			visit(body);
		dedent();
	}
}

std::ostream & ASTDumper::getOut(const uint8_t& num)
{
	out_ << offset_ << getIndent(num);
	return out_;
}

void ASTDumper::recalculateOffset()
{
	offset_ = "";
	for (auto idx = offsetTabs_; idx > 0; idx--)
		offset_ += OFFSET_INDENT;
}

std::string ASTDumper::getIndent(const uint8_t& num) const
{
	auto totalIndent = curIndent_ + num;
	if (totalIndent)
	{
		std::string rtr;
		for (auto k = totalIndent; k > 0; --k)
			rtr += INDENT; // indent is 2 spaces

		rtr += u8"┗";
		return rtr;
	}
	return "";
}

std::string ASTDumper::getStmtNodeName(Stmt* stmt) const
{
	switch (stmt->getKind())
	{
		#define STMT(ID,PARENT) case StmtKind::ID: return #ID;
		#include "StmtNodes.def"
		default:
			throw std::exception("Unreachable");
	}
}

std::string ASTDumper::getDeclNodeName(Decl * decl) const
{
	switch (decl->getKind())
	{
		#define DECL(ID,PARENT) case DeclKind::ID: return #ID;
		#include "DeclNodes.def"
		default:
			throw std::exception("Unreachable");
	}
}

std::string ASTDumper::getTypeNodeName(Type * type) const
{
	switch (type->getKind())
	{
		#define TYPE(ID,PARENT) case TypeKind::ID: return #ID;
		#include "TypeNodes.def"
		default:
			throw std::exception("Unreachable");
	}
}

std::string ASTDumper::getBasicStmtInfo(Stmt * stmt) const
{
	std::ostringstream ss;
	ss << getStmtNodeName(stmt) << " " << (void *)stmt;
	return ss.str();
}

std::string ASTDumper::getBasicDeclInfo(Decl * decl) const
{
	std::ostringstream ss;
	ss << getDeclNodeName(decl) << " " << (void *)decl;
	return ss.str();
}

std::string ASTDumper::getBasicTypeInfo(Type * type) const
{
	std::ostringstream ss;
	ss << getTypeNodeName(type) << " " << (void *)type;
	return ss.str();
}

void ASTDumper::indent(const uint8_t & num)
{
	curIndent_ += num;
}

void ASTDumper::dedent(const uint8_t & num)
{
	if (curIndent_)
	{
		if (curIndent_ >= num)
			curIndent_ -= num;
		else
			curIndent_ = 0;
	}
}
