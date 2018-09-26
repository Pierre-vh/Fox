////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTDumper.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Fox/AST/ASTDumper.hpp"
#include "Fox/Common/Source.hpp"
#include "Fox/AST/Identifiers.hpp"
#include "Fox/Common/StringManipulator.hpp"
#include "Fox/Common/Errors.hpp"
#include <string>
#include <sstream>

#define INDENT "    "
#define OFFSET_INDENT "\t"

using namespace fox;

ASTDumper::ASTDumper(SourceManager& srcMgr,std::ostream & out, const uint8_t & offsettabs) : out_(out), offsetTabs_(offsettabs), srcMgr_(srcMgr)
{
	recalculateOffset();
}

void ASTDumper::visitParensExpr(ParensExpr* node)
{
	if (auto expr = node->getExpr())
	{
		auto range = node->getRange();
		dumpLine() << getBasicExprInfo(node) << " " << getSourceLocDump("LParen", range.getBegin()) << " " << getSourceLocDump("RParen", range.getEnd()) << "\n";
		indent();
			visit(expr);
		dedent();
	}
	else
		dumpLine() << getBasicExprInfo(node) << " (Empty)\n";
}

void ASTDumper::visitBinaryExpr(BinaryExpr* node)
{
	dumpLine() << getBasicExprInfo(node) << " " << getOperatorDump(node->getOp()) << "\n";

	// Print LHS 
	indent();
		visit(node->getLHS());
	dedent();

	// Print RHS
	indent();
		visit(node->getRHS());
	dedent();
}

void ASTDumper::visitCastExpr(CastExpr* node)
{
	dumpLine() << getBasicExprInfo(node) << " " << getTypeDump("to", node->getCastTypeLoc()) << "\n";
	indent();
		visit(node->getExpr());
	dedent();
}

void ASTDumper::visitUnaryExpr(UnaryExpr* node)
{
	dumpLine() << getBasicExprInfo(node) << " " << getOperatorDump(node->getOp()) << "\n";
	indent();
		visit(node->getExpr());
	dedent();
}

void ASTDumper::visitArrayAccessExpr(ArrayAccessExpr* node)
{
	dumpLine() << getBasicExprInfo(node) << '\n';

	indent();
		visit(node->getExpr());
	dedent();

	// Print IdxExpr
	indent();
		visit(node->getIdxExpr());
	dedent();
}

void ASTDumper::visitMemberOfExpr(MemberOfExpr* node)
{
	dumpLine() << getBasicExprInfo(node) << " " << getIdentifierDump(node->getMemberID()) << "\n";
	indent();
		visit(node->getExpr());
	dedent();
}

void ASTDumper::visitDeclRefExpr(DeclRefExpr* node)
{
	dumpLine() << getBasicExprInfo(node) << " " << getIdentifierDump(node->getIdentifier()) << "\n";
}

void ASTDumper::visitFunctionCallExpr(FunctionCallExpr* node)
{
	dumpLine() << getBasicExprInfo(node) << '\n';

	// Print Base 
	indent();
		visit(node->getCallee());
	dedent();

	// Print Args
	for (Expr* arg: node->getArgs())
	{
		indent();
			visit(arg);
		dedent();
	}
}

void ASTDumper::visitCharLiteralExpr(CharLiteralExpr* node)
{
	std::string res;
	StringManipulator::append(res, node->getVal());
	dumpLine() << getBasicExprInfo(node) << " " << makeKeyPairDump("value",addSingleQuotes(res)) << "\n";
}

void ASTDumper::visitIntegerLiteralExpr(IntegerLiteralExpr* node)
{
	dumpLine() << getBasicExprInfo(node) << " " << makeKeyPairDump("value", node->getVal()) << "\n";
}

void ASTDumper::visitFloatLiteralExpr(FloatLiteralExpr* node)
{
	dumpLine() << getBasicExprInfo(node) << " " << makeKeyPairDump("value", node->getVal()) << "\n";
}

void ASTDumper::visitBoolLiteralExpr(BoolLiteralExpr* node)
{
	dumpLine() << getBasicExprInfo(node) << " " << makeKeyPairDump("value", (node->getVal() ? "true" : "false" )) << "\n";
}

void ASTDumper::visitStringLiteralExpr(StringLiteralExpr* node)
{
	dumpLine() << getBasicExprInfo(node) << " " << makeKeyPairDump("value", addDoubleQuotes(node->getVal())) << "\n";
}

void ASTDumper::visitArrayLiteralExpr(ArrayLiteralExpr* node)
{
	std::size_t elemcount = node->getSize();

	dumpLine() << getBasicExprInfo(node) << " " << makeKeyPairDump("size", elemcount) << "\n";

	for (Expr* expr : node->getExprs())
	{
		indent();
			visit(expr);
		dedent();
	}
}

void ASTDumper::visitNullStmt(NullStmt* node)
{
	dumpLine() << getBasicStmtInfo(node) << "\n";
}

void ASTDumper::visitCompoundStmt(CompoundStmt* node)
{
	dumpLine() << getBasicStmtInfo(node) << '\n';
	indent();
	for (auto it = node->nodes_begin(); it != node->nodes_end(); it++)
		visit(*it);
	dedent();
}

void ASTDumper::visitConditionStmt(ConditionStmt* node)
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

void ASTDumper::visitWhileStmt(WhileStmt* node)
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

void ASTDumper::visitReturnStmt(ReturnStmt* node)
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
	dumpLine() << getBasicValueDeclDump(node) << " ";
	if (node->hasInitExpr())
	{
		indent(1);
			visit(node->getInitExpr());
		dedent(1);
	}
}

void ASTDumper::visitParamDecl(ParamDecl* node)
{
	dumpLine() << getBasicValueDeclDump(node) << "\n";
}

void ASTDumper::visitFuncDecl(FuncDecl* node)
{
	dumpLine() << getBasicDeclInfo(node) << " " << getIdentifierDump(node->getIdentifier()) << " " << getTypeDump("returns",node->getReturnType()) << " " << getDeclRecorderDump(node) << "\n";

	if (node->getNumParams())
	{
		unsigned counter = 0;
		for (auto it = node->params_begin(); it != node->params_end(); it++, counter++)
		{
			indent();
				visitParamDecl(*it);
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

void ASTDumper::setPrintAllAddresses(bool opt)
{
	printAllAdresses_ = opt;
}

bool ASTDumper::getPrintAllAddresses() const
{
	return printAllAdresses_;
}

void ASTDumper::initDefaultOptions()
{
	// currently it's hard coded defaults
	printAllAdresses_ = false;
}

std::ostream & ASTDumper::dumpLine(std::uint8_t num)
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
		#include "Fox/AST/StmtNodes.def"
		default:
			fox_unreachable("unknown node");
	}
}

std::string ASTDumper::getExprNodeName(Expr* expr) const
{
	switch (expr->getKind())
	{
		#define EXPR(ID,PARENT) case ExprKind::ID: return #ID;
		#include "Fox/AST/ExprNodes.def"
		default:
			fox_unreachable("unknown node");
	}
}

std::string ASTDumper::getDeclNodeName(Decl* decl) const
{
	switch (decl->getKind())
	{
		#define DECL(ID,PARENT) case DeclKind::ID: return #ID;
		#include "Fox/AST/DeclNodes.def"
		default:
			fox_unreachable("unknown node");
	}
}

std::string ASTDumper::getTypeNodeName(TypeBase* type) const
{
	switch (type->getKind())
	{
		#define TYPE(ID,PARENT) case TypeKind::ID: return #ID;
		#include "Fox/AST/TypeNodes.def"
		default:
			fox_unreachable("unknown node");
	}
}

std::string ASTDumper::getBasicStmtInfo(Stmt* stmt) const
{
	std::ostringstream ss;
	ss << getStmtNodeName(stmt);
	if (printAllAdresses_)
		ss << " " << (void *)stmt;
	return ss.str();
}

std::string ASTDumper::getBasicExprInfo(Expr* expr) const
{
	std::ostringstream ss;
	ss << getExprNodeName(expr);
	if (printAllAdresses_)
		ss << " " << (void *)expr;
	if (auto ty = expr->getType())
		ss << " " << makeKeyPairDump("type", ty->getString());
	return ss.str();
}

std::string ASTDumper::getBasicDeclInfo(Decl* decl) const
{
	std::ostringstream ss;
	ss << getDeclNodeName(decl);
	if (printAllAdresses_)
		ss << " " << (void *)decl;

	SourceRange range = decl->getRange();
	ss << " " << getSourceLocDump("start", range.getBegin());
	ss << " " << getSourceLocDump("end", range.getEnd());

	return ss.str();
}

std::string ASTDumper::getBasicTypeInfo(TypeBase* type) const
{
	std::ostringstream ss;
	ss << getTypeNodeName(type);
	if (printAllAdresses_)
		ss << " " << (void *)type;
	return ss.str();
}

std::string ASTDumper::getBasicValueDeclDump(ValueDecl* decl) const
{
	std::ostringstream ss;
	ss << getDeclNodeName(decl);
	if (printAllAdresses_)
		ss << " " << (void *)decl;

	SourceRange range = decl->getRange();
	ss << " " << getSourceLocDump("start", range.getBegin()) << " ";
	ss << getSourceLocDump("end", range.getEnd());

	ss << makeKeyPairDump("id", decl->getIdentifier()->getStr()) << " ";
	ss << makeKeyPairDump("type", decl->getTypeLoc()->getString()) << " ";

	if (decl->isConstant())
		ss << "const ";

	SourceRange typeRange = decl->getTypeLoc().getRange();
	ss << getSourceLocDump("type start", typeRange.getBegin()) << " ";
	ss << getSourceLocDump("type end" , typeRange.getEnd());

	return ss.str();
}

std::string ASTDumper::getOperatorDump(BinaryExpr::OpKind op) const
{
	std::ostringstream ss;
	ss << BinaryExpr::getOpSign(op) << " (" << BinaryExpr::getOpName(op) << ")";
	return ss.str();
}

std::string ASTDumper::getOperatorDump(UnaryExpr::OpKind op) const
{
	std::ostringstream ss;
	ss << UnaryExpr::getOpSign(op) << " (" << UnaryExpr::getOpName(op) << ")";
	return ss.str();
}

std::string ASTDumper::getDeclRecorderDump(DeclContext* dr) const
{
	std::ostringstream ss;
	ss << "<DeclContext:" << (void*)dr;
	if (dr->hasParentDeclRecorder())
		ss << ", Parent:" << (void*)dr->getParentDeclRecorder();
	ss << ">";
	return ss.str();
}

std::string ASTDumper::getIdentifierDump(Identifier* id) const
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

std::string ASTDumper::getTypeDump(const std::string& label, Type ty) const
{
	return makeKeyPairDump(label, addSingleQuotes(ty->getString()));
}

std::string ASTDumper::addDoubleQuotes(const std::string& str) const
{
	return "\"" + str + "\"";
}

std::string ASTDumper::addSingleQuotes(const std::string& str) const
{
	return "'" + str + "'";
}

void ASTDumper::indent(std::uint8_t num)
{
	curIndent_ += num;
}

void ASTDumper::dedent(std::uint8_t num)
{
	if (curIndent_)
	{
		if (curIndent_ >= num)
			curIndent_ -= num;
		else
			curIndent_ = 0;
	}
}