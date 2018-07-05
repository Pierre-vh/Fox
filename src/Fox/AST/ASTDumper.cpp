////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTDumper.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "ASTDumper.hpp"
#include "Fox/Common/Source.hpp"
#include "Identifiers.hpp"
#include "Fox/Common/StringManipulator.hpp"
#include <string>
#include <sstream>

#define INDENT "    "
#define OFFSET_INDENT "\t"

using namespace fox;

ASTDumper::ASTDumper(SourceManager& srcMgr,std::ostream & out, const uint8_t & offsettabs) : out_(out), offsetTabs_(offsettabs), srcMgr_(srcMgr)
{
	recalculateOffset();
}

void ASTDumper::visitParensExpr(ParensExpr * node)
{
	if (auto expr = node->getExpr())
	{
		dumpLine() << getBasicStmtInfo(node) << " " << getSourceLocDump("LParen", node->getBegLoc()) << " " << getSourceLocDump("RParen", node->getEndLoc()) << "\n";
		indent();
			visit(expr);
		dedent();
	}
	else
		dumpLine() << getBasicStmtInfo(node) << " (Empty)\n";
}

void ASTDumper::visitBinaryExpr(BinaryExpr * node)
{
	dumpLine() << getBasicStmtInfo(node) << " " << getOperatorDump(node->getOp()) << "\n";

	// Print LHS 
	indent();
		visit(node->getLHS());
	dedent();

	// Print RHS
	indent();
		visit(node->getRHS());
	dedent();
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

	indent();
		visit(node->getBase());
	dedent();

	// Print IdxExpr
	indent();
		visit(node->getAccessIndexExpr());
	dedent();
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
	indent();
		visit(node->getCallee());
	dedent();

	// Print Args if there are args
	if (node->getExprList() && (!node->getExprList()->isEmpty()))
	{
		auto elist = node->getExprList();
		for (auto it = elist->begin(); it != elist->end(); it++)
		{
			indent();
				visit(*it);
			dedent();
		}
	}
}

void ASTDumper::visitCharLiteralExpr(CharLiteralExpr * node)
{
	std::string res;
	StringManipulator::append(res, node->getVal());
	dumpLine() << getBasicStmtInfo(node) << " " << makeKeyPairDump("value",addSingleQuotes(res)) << "\n";
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
	dumpLine() << getBasicStmtInfo(node) << " " << makeKeyPairDump("value", addDoubleQuotes(node->getVal())) << "\n";
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
		for (auto it = elist->begin(); it != elist->end(); it++)
		{
			indent();
				visit(*it);
			dedent();
		}
	}
}

void ASTDumper::visitNullStmt(NullStmt * node)
{
	dumpLine() << getBasicStmtInfo(node) << "\n";
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
	indent();
		visit(node->getCond());
	dedent();

	// Visit Then
	indent();
		visit(node->getThen());
	dedent();

	// If there's a else, visit it
	if (node->hasElse())
	{
		indent();
			visit(node->getElse());
		dedent();
	}
}

void ASTDumper::visitWhileStmt(WhileStmt * node)
{
	dumpLine() << getBasicStmtInfo(node) << "\n";
	// Visit cond
	indent();
		visit(node->getCond());
	dedent();

	// Visit body
	indent();
		visit(node->getBody());
	dedent();
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
		indent();
			visit(node->getExpr());
		dedent();
	}
}

void ASTDumper::visitUnitDecl(UnitDecl* node)
{
	std::string fileInfo;
	if (const auto* data = srcMgr_.getStoredDataForFileID(node->getFileID()))
		fileInfo = makeKeyPairDump("file", data->fileName);
	else
		fileInfo = makeKeyPairDump("file", "unknown");

	dumpLine() << getBasicDeclInfo(node) << " " 
		<< fileInfo << " " 
		<< getIdentifierDump(node->getIdentifier()) << " " 
		<< getDeclRecorderDump(node) << "\n";

	indent();
	for (auto it = node->decls_beg(); it != node->decls_end(); it++)
		visit(*it);
	dedent();
}

void ASTDumper::visitVarDecl(VarDecl* node)
{
	dumpLine() << getBasicDeclInfo(node) << " "
		<< getIdentifierDump(node->getIdentifier()) << " " 
		<< getQualTypeDump("type",node->getType()) << "\n";
	if (node->hasInitExpr())
	{
		indent(1);
			visit(node->getInitExpr());
		dedent(1);
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
		unsigned counter = 0;
		for (auto it = node->args_begin(); it != node->args_end(); it++, counter++)
		{
			indent();
				visitArgDecl(*it);
			dedent();
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
			fox_unreachable("unknown node");
	}
}

std::string ASTDumper::getDeclNodeName(Decl * decl) const
{
	switch (decl->getKind())
	{
		#define DECL(ID,PARENT) case DeclKind::ID: return #ID;
		#include "DeclNodes.def"
		default:
			fox_unreachable("unknown node");
	}
}

std::string ASTDumper::getTypeNodeName(Type * type) const
{
	switch (type->getKind())
	{
		#define TYPE(ID,PARENT) case TypeKind::ID: return #ID;
		#include "TypeNodes.def"
		default:
			fox_unreachable("unknown node");
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

	ss << " " << getSourceLocDump("start", decl->getBegLoc());
	ss << " " << getSourceLocDump("end", decl->getEndLoc());

	return ss.str();
}

std::string ASTDumper::getBasicTypeInfo(Type * type) const
{
	std::ostringstream ss;
	ss << getTypeNodeName(type);
	if (printAllAdresses_)
		ss << " " << (void *)type;
	return ss.str();
}

std::string ASTDumper::getOperatorDump(const BinaryOperator & op) const
{
	if (dumpOperatorsAsNames_)
		return "'" + operators::getName(op) + "'";
	return "'" + operators::toString(op) + "'";
}

std::string ASTDumper::getOperatorDump(const UnaryOperator & op) const
{
	if (dumpOperatorsAsNames_)
		return "'" + operators::getName(op) + "'";
	return "'" + operators::toString(op) + "'";
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
	return makeKeyPairDump("id", addSingleQuotes(id->getStr()));
}

std::string ASTDumper::getSourceLocDump(const std::string& label,const SourceLoc& sloc) const
{
	std::ostringstream ss;
	if (sloc)
	{
		CompleteLoc cloc = srcMgr_.getCompleteLocForSourceLoc(sloc);
		ss << "[l" << cloc.line << ",c" << cloc.column << "]";
	}
	else
		ss << "[invalid sloc]";

	return makeKeyPairDump(label, ss.str());
}

std::string ASTDumper::getTypeDump(const std::string & label, Type * ty) const
{
	return makeKeyPairDump(label, addSingleQuotes(ty->getString()));
}

std::string ASTDumper::getQualTypeDump(const std::string & label, const QualType & qt) const
{
	return makeKeyPairDump(label, addSingleQuotes(qt.getString()));
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