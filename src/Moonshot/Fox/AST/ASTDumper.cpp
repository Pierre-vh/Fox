////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTDumper.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "ASTDumper.hpp"
#include "Identifiers.hpp"

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
	dumpLine() << getBasicStmtInfo(node) << " " << getOperatorDump(node->getOp()) << "\n";

	// Print LHS 
	dumpLine(1) << "[LHS]\n";
	indent(2);
		visit(node->getLHS());
	dedent(2);

	// Print RHS
	dumpLine(1) << "[RHS]\n";
	indent(2);
		visit(node->getRHS());
	dedent(2);
}

void ASTDumper::visitCastExpr(CastExpr * node)
{
	dumpLine() << getBasicStmtInfo(node) << " " << getTypeDump("to",node->getCastGoal()) << "\n";
	indent();
		visit(node->getChild());
	dedent();
}

void ASTDumper::visitUnaryExpr(UnaryExpr * node)
{
	dumpLine() << getBasicStmtInfo(node) << " " << getOperatorDump(node->getOp()) << "\n";
	indent();
		visit(node->getChild());
	dedent();
}

void ASTDumper::visitArrayAccessExpr(ArrayAccessExpr * node)
{
	dumpLine() << getBasicStmtInfo(node) << '\n';

	// Print Base 
	dumpLine(1) << "[Base]\n";
	indent(2);
		visit(node->getBase());
	dedent(2);

	// Print IdxExpr
	dumpLine(1) << "[Index]\n";
	indent(2);
		visit(node->getAccessIndexExpr());
	dedent(2);
}

void ASTDumper::visitMemberOfExpr(MemberOfExpr * node)
{
	dumpLine() << getBasicStmtInfo(node) << " " << getIdentifierDump(node->getMemberID()) << "\n";
	indent();
		visit(node->getBase());
	dedent();
}

void ASTDumper::visitDeclRefExpr(DeclRefExpr * node)
{
	dumpLine() << getBasicStmtInfo(node) << " " << getIdentifierDump(node->getIdentifier()) << "\n";
}

void ASTDumper::visitFunctionCallExpr(FunctionCallExpr * node)
{
	dumpLine() << getBasicStmtInfo(node) << '\n';

	// Print Base 
	dumpLine(1) << "[Callee]\n";
	indent(2);
		visit(node->getCallee());
	dedent(2);

	// Print Args if there are args
	if (node->getExprList() && (!node->getExprList()->isEmpty()))
	{
		dumpLine(1) << "[Args]\n";
		auto elist = node->getExprList();
		unsigned counter = 0;
		for (auto it = elist->begin(); it != elist->end(); it++, counter++)
		{
			dumpLine(2) << "[Arg " << counter << "]\n";
			indent(3);
				visit(*it);
			dedent(3);
		}
	}
}

void ASTDumper::visitNullExpr(NullExpr * node)
{
	dumpLine() << getBasicStmtInfo(node) << "\n";
}

void ASTDumper::visitCharLiteralExpr(CharLiteralExpr * node)
{
	dumpLine() << getBasicStmtInfo(node) << " " << makeKeyPairDump("value",node->getVal()) << "\n";
}

void ASTDumper::visitIntegerLiteralExpr(IntegerLiteralExpr * node)
{
	dumpLine() << getBasicStmtInfo(node) << " " << makeKeyPairDump("value", node->getVal()) << "\n";
}

void ASTDumper::visitFloatLiteralExpr(FloatLiteralExpr * node)
{
	dumpLine() << getBasicStmtInfo(node) << " " << makeKeyPairDump("value", node->getVal()) << "\n";
}

void ASTDumper::visitBooleanLiteralExpr(BoolLiteralExpr * node)
{
	dumpLine() << getBasicStmtInfo(node) << " " << makeKeyPairDump("value", (node->getVal() ? "true" : "false" )) << "\n";
}

void ASTDumper::visitStringLiteralExpr(StringLiteralExpr * node)
{
	dumpLine() << getBasicStmtInfo(node) << " " << makeKeyPairDump("value", node->getVal()) << "\n";
}

void ASTDumper::visitArrayLiteralExpr(ArrayLiteralExpr * node)
{
	std::size_t elemcount = 0;
	if (node->hasExprList())
		elemcount = node->getExprList()->size();

	dumpLine() << getBasicStmtInfo(node) << " " << makeKeyPairDump("size",elemcount) << "\n";

	if (node->hasExprList())
	{
		ExprList* elist = node->getExprList();
		uint16_t counter = 0;
		for (auto it = elist->begin(); it != elist->end(); it++)
		{
			dumpLine(1) << "[" << counter << "]\n";
			indent(2);
				visit(*it);
			dedent(2);
			counter++;
		}
	}
}

void ASTDumper::visitCompoundStmt(CompoundStmt * node)
{
	dumpLine() << getBasicStmtInfo(node) << '\n';
	indent();
	for (auto it = node->stmts_beg(); it != node->stmts_end(); it++)
		visit(*it);
	dedent();
}

void ASTDumper::visitConditionStmt(ConditionStmt * node)
{
	dumpLine() << getBasicStmtInfo(node) << "\n";
	// Visit cond
	dumpLine(1) << "[Cond Expr]\n";
	indent(2);
		visit(node->getCond());
	dedent(2);

	// Visit Then
	dumpLine(1) << "[Then]\n";
	indent(2);
		visit(node->getThen());
	dedent(2);

	// If there's a else, visit it
	if (node->hasElse())
	{
		dumpLine(1) << "[Else]\n";
		indent(2);
			visit(node->getElse());
		dedent(2);
	}
}

void ASTDumper::visitWhileStmt(WhileStmt * node)
{
	dumpLine() << getBasicStmtInfo(node) << "\n";
	// Visit cond
	dumpLine(1) << "[Cond Expr]\n";
	indent(2);
		visit(node->getCond());
	dedent(2);

	// Visit body
	dumpLine(1) << "[Body]\n";
	indent(2);
		visit(node->getBody());
	dedent(2);
}

void ASTDumper::visitDeclStmt(DeclStmt * node)
{
	dumpLine() << getBasicStmtInfo(node) << "\n";
	indent();
		visit(node->getDecl());
	dedent();
}

void ASTDumper::visitReturnStmt(ReturnStmt * node)
{
	dumpLine() << getBasicStmtInfo(node) << "\n";
	if (node->hasExpr())
	{
		dumpLine(1) << "[Expr]\n";
		indent(2);
			visit(node->getExpr());
		dedent(2);
	}
}

void ASTDumper::visitUnitDecl(UnitDecl * node)
{
	dumpLine() << getBasicDeclInfo(node) << " " << getIdentifierDump(node->getIdentifier()) << " " << getDeclRecorderDump(node) << "\n";

	indent();
	for (auto it = node->decls_beg(); it != node->decls_end(); it++)
		visit(*it);
	dedent();
}

void ASTDumper::visitVarDecl(VarDecl * node)
{
	dumpLine() << getBasicDeclInfo(node) << " " << getIdentifierDump(node->getIdentifier()) << " " << getQualTypeDump("type",node->getType()) << "\n";
	if (node->hasInitExpr())
	{
		dumpLine(1) << "[Init]\n";
		indent(2);
			visit(node->getInitExpr());
		dedent(2);
	}
}

void ASTDumper::visitArgDecl(ArgDecl * node)
{
	dumpLine() << getBasicDeclInfo(node) << " " << getIdentifierDump(node->getIdentifier()) << " " << getQualTypeDump("type", node->getType()) << "\n";
}

void ASTDumper::visitFunctionDecl(FunctionDecl * node)
{
	dumpLine() << getBasicDeclInfo(node) << " " << getIdentifierDump(node->getIdentifier()) << " " << getTypeDump("returns",node->getReturnType()) << " " << getDeclRecorderDump(node) << "\n";

	if (node->argsSize())
	{
		dumpLine(1) << "[Args Decls]\n";
		unsigned counter = 0;
		for (auto it = node->args_begin(); it != node->args_end(); it++, counter++)
		{
			dumpLine(2) << "[Arg " << counter << "]\n";
			indent(3);
				visit(*it);
			dedent(3);
		}
	}
	// Visit the compound statement
	dumpLine() << "[Body]\n";
	if (auto body = node->getBody())
	{
		indent(2);
			visit(body);
		dedent(2);
	}
}

void ASTDumper::setPrintAllAddresses(const bool & opt)
{
	printAllAdresses_ = opt;
}

bool ASTDumper::getPrintAllAddresses() const
{
	return printAllAdresses_;
}

void ASTDumper::setDumpOperatorsAsNames(const bool & opt)
{
	dumpOperatorsAsNames_ = opt;
}

bool ASTDumper::getDumpOperatorsAsNames() const
{
	return dumpOperatorsAsNames_;
}

void ASTDumper::initDefaultOptions()
{
	// currently it's hard coded defaults
	printAllAdresses_ = false;
	dumpOperatorsAsNames_ = false;
}

std::ostream & ASTDumper::dumpLine(const uint8_t& num)
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
	ss << getStmtNodeName(stmt);
	if (printAllAdresses_)
		ss << " " << (void *)stmt;
	return ss.str();
}

std::string ASTDumper::getBasicDeclInfo(Decl * decl) const
{
	std::ostringstream ss;
	ss << getDeclNodeName(decl);
	if (printAllAdresses_)
		ss << " " << (void *)decl;
	return ss.str();
}

std::string ASTDumper::getBasicTypeInfo(Type * type) const
{
	std::ostringstream ss;
	if (printAllAdresses_)
		ss << " " << (void *)type;
	return ss.str();
}

std::string ASTDumper::getOperatorDump(const binaryOperator & op) const
{
	if (dumpOperatorsAsNames_)
		return "'" + Operators::getName(op) + "'";
	return "'" + Operators::toString(op) + "'";
}

std::string ASTDumper::getOperatorDump(const unaryOperator & op) const
{
	if (dumpOperatorsAsNames_)
		return "'" + Operators::getName(op) + "'";
	return "'" + Operators::toString(op) + "'";
}

std::string ASTDumper::getDeclRecorderDump(DeclRecorder * dr) const
{
	std::ostringstream ss;
	ss << "<DeclRecorder:" << (void*)dr;
	if (dr->hasParentDeclRecorder())
		ss << ", Parent:" << (void*)dr->getParentDeclRecorder();
	ss << ">";
	return ss.str();
}

std::string ASTDumper::getIdentifierDump(IdentifierInfo * id) const
{
	std::ostringstream ss;
	ss << "<id:'" << id->getStr() << "'>";
	return ss.str();
}

std::string ASTDumper::getTypeDump(const std::string & label, Type * ty) const
{
	std::ostringstream ss;
	ss << "<" << label << ":'" << ty->getString() << "'>";
	return ss.str();
}

std::string ASTDumper::getQualTypeDump(const std::string & label, const QualType & qt) const
{
	std::ostringstream ss;
	ss << "<" << label << ":'" << qt.getString() << "'>";
	return ss.str();
}

std::string ASTDumper::addDoubleQuotes(const std::string & str) const
{
	return "\"" + str + "\"";
}

std::string ASTDumper::addSingleQuotes(const std::string & str) const
{
	return "'" + str + "'";
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
