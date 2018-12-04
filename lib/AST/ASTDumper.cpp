//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.
// See LICENSE.txt for license info.
// File : ASTDumper.cpp
// Author : Pierre van Houtryve
//----------------------------------------------------------------------------//

#include "Fox/AST/ASTDumper.hpp"
#include "Fox/AST/Identifiers.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/Common/Source.hpp"
#include "Fox/Common/StringManipulator.hpp"

#include <sstream>
#include <iostream>
#include <string>

#define INDENT "    "
#define OFFSET_INDENT "\t"

using namespace fox;

ASTDumper::ASTDumper(SourceManager& srcMgr,
                     std::ostream& out,
                     const uint8_t& offsettabs)
    : out_(out), offsetTabs_(offsettabs), srcMgr_(&srcMgr) {
  recalculateOffset();
}

ASTDumper::ASTDumper(std::ostream& out, const uint8_t & offsettabs):
  srcMgr_(nullptr), out_(out), offsetTabs_(offsettabs) {

}

void ASTDumper::visitBinaryExpr(BinaryExpr* node) {
  dumpLine() << getBasicExprInfo(node) << " " << getOperatorDump(node)
             << "\n";

  // Print LHS
  indent();
  visit(node->getLHS());
  dedent();

  // Print RHS
  indent();
  visit(node->getRHS());
  dedent();
}

void ASTDumper::visitCastExpr(CastExpr* node) {
  dumpLine() << getBasicExprInfo(node) << " "
             << getTypeLocDump("to", node->getCastTypeLoc()) << "\n";
  indent();
  visit(node->getExpr());
  dedent();
}

void ASTDumper::visitUnaryExpr(UnaryExpr* node) {
  dumpLine() << getBasicExprInfo(node) << " " << getOperatorDump(node)
             << "\n";
  indent();
  visit(node->getExpr());
  dedent();
}

void ASTDumper::visitArraySubscriptExpr(ArraySubscriptExpr* node) {
  dumpLine() << getBasicExprInfo(node) << '\n';

  indent();
  visit(node->getBase());
  dedent();

  // Print IdxExpr
  indent();
  visit(node->getIndex());
  dedent();
}

void ASTDumper::visitMemberOfExpr(MemberOfExpr* node) {
  dumpLine() << getBasicExprInfo(node) << " "
             << getIdentifierDump(node->getMemberID()) << "\n";
  indent();
  visit(node->getExpr());
  dedent();
}

void ASTDumper::visitDeclRefExpr(DeclRefExpr* node) {
  dumpLine() << getBasicExprInfo(node) << " "
             << getIdentifierDump(node->getIdentifier()) << "\n";
}

void ASTDumper::visitFunctionCallExpr(FunctionCallExpr* node) {
  dumpLine() << getBasicExprInfo(node) << '\n';

  // Print Base
  indent();
  visit(node->getCallee());
  dedent();

  // Print Args
  for (Expr* arg : node->getArgs()) {
    indent();
    visit(arg);
    dedent();
  }
}

void ASTDumper::visitCharLiteralExpr(CharLiteralExpr* node) {
  std::string res;
  StringManipulator::append(res, node->getVal());
  dumpLine() << getBasicExprInfo(node) << " "
             << makeKeyPairDump("value", addSingleQuotes(res)) << "\n";
}

void ASTDumper::visitIntegerLiteralExpr(IntegerLiteralExpr* node) {
  dumpLine() << getBasicExprInfo(node) << " "
             << makeKeyPairDump("value", node->getVal()) << "\n";
}

void ASTDumper::visitFloatLiteralExpr(FloatLiteralExpr* node) {
  dumpLine() << getBasicExprInfo(node) << " "
             << makeKeyPairDump("value", node->getVal()) << "\n";
}

void ASTDumper::visitBoolLiteralExpr(BoolLiteralExpr* node) {
  dumpLine() << getBasicExprInfo(node) << " "
             << makeKeyPairDump("value", (node->getVal() ? "true" : "false"))
             << "\n";
}

void ASTDumper::visitStringLiteralExpr(StringLiteralExpr* node) {
  dumpLine() << getBasicExprInfo(node) << " "
             << makeKeyPairDump("value", addDoubleQuotes(node->getVal()))
             << "\n";
}

void ASTDumper::visitArrayLiteralExpr(ArrayLiteralExpr* node) {
  std::size_t elemcount = node->getSize();

  dumpLine() << getBasicExprInfo(node) << " "
             << makeKeyPairDump("size", elemcount) << "\n";

  for (Expr* expr : node->getExprs()) {
    indent();
    visit(expr);
    dedent();
  }
}

void ASTDumper::visitNullStmt(NullStmt* node) {
  dumpLine() << getBasicStmtInfo(node) << "\n";
}

void ASTDumper::visitCompoundStmt(CompoundStmt* node) {
  dumpLine() << getBasicStmtInfo(node) << '\n';
  indent();
  for (auto it = node->nodes_begin(); it != node->nodes_end(); it++)
    visit(*it);
  dedent();
}

void ASTDumper::visitConditionStmt(ConditionStmt* node) {
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
  if (node->hasElse()) {
    indent();
    visit(node->getElse());
    dedent();
  }
}

void ASTDumper::visitWhileStmt(WhileStmt* node) {
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

void ASTDumper::visitReturnStmt(ReturnStmt* node) {
  dumpLine() << getBasicStmtInfo(node) << "\n";
  if (node->hasExpr()) {
    indent();
    visit(node->getExpr());
    dedent();
  }
}

void ASTDumper::visitUnitDecl(UnitDecl* node) {
  std::string fileInfo;
  if (const auto* data = getSourceData(node->getFileID()))
    fileInfo = makeKeyPairDump("file", data->fileName);
  else
    fileInfo = makeKeyPairDump("file", "unknown");

  dumpLine() << getBasicDeclInfo(node) << " " << fileInfo << " "
             << getIdentifierDump(node->getIdentifier()) << " "
             << getDeclRecorderDump(node) << "\n";

  indent();
  for (auto it = node->decls_beg(); it != node->decls_end(); it++)
    visit(*it);
  dedent();
}

void ASTDumper::visitVarDecl(VarDecl* node) {
  dumpLine() << getBasicValueDeclDump(node) << "\n";
  if (node->hasInitExpr()) {
    indent(1);
    visit(node->getInitExpr());
    dedent(1);
  }
}

void ASTDumper::visitParamDecl(ParamDecl* node) {
  dumpLine() << getBasicValueDeclDump(node) << "\n";
}

void ASTDumper::visitFuncDecl(FuncDecl* node) {
  dumpLine() << getBasicDeclInfo(node) << " "
             << getIdentifierDump(node->getIdentifier()) << " "
             << getTypeLocDump("returns", node->getReturnTypeLoc()) << " "
             << getDeclRecorderDump(node) << "\n";

  if (node->getNumParams()) {
    unsigned counter = 0;
    for (auto it = node->params_begin(); it != node->params_end();
         it++, counter++) {
      indent();
      visitParamDecl(*it);
      dedent();
    }
  }
  // Visit the compound statement
  if (auto body = node->getBody()) {
    indent();
    visit(body);
    dedent();
  }
}

void ASTDumper::visit(Type type) {
  dumpLine() << toString(type);
}

bool ASTDumper::isDebug() const {
  return debug_;
}

std::string ASTDumper::toString(Type type) const {
  return isDebug() ? type->toDebugString() : type->toString();
}

const SourceManager::SourceData* ASTDumper::getSourceData(FileID fid) {
  if (srcMgr_)
    return srcMgr_->getSourceData(fid);
  return nullptr;
}

bool ASTDumper::hasSrcMgr() const {
  return (bool)srcMgr_;
}

std::ostream& ASTDumper::dumpLine(std::uint8_t num) {
  out_ << offset_ << getIndent(num);
  return out_;
}

void ASTDumper::recalculateOffset() {
  offset_ = "";
  for (auto idx = offsetTabs_; idx > 0; idx--)
    offset_ += OFFSET_INDENT;
}

std::string ASTDumper::getIndent(const uint8_t& num) const {
  auto totalIndent = curIndent_ + num;
  if (totalIndent) {
    std::string rtr;
    for (auto k = totalIndent; k > 0; --k)
      rtr += INDENT;

    rtr += u8"┗";
    return rtr;
  }
  return "";
}

std::string ASTDumper::getStmtNodeName(Stmt* stmt) const {
  switch (stmt->getKind()) {
#define STMT(ID, PARENT) \
  case StmtKind::ID:     \
    return #ID;
#include "Fox/AST/StmtNodes.def"
    default:
      fox_unreachable("unknown node");
  }
}

std::string ASTDumper::getExprNodeName(Expr* expr) const {
  switch (expr->getKind()) {
#define EXPR(ID, PARENT) \
  case ExprKind::ID:     \
    return #ID;
#include "Fox/AST/ExprNodes.def"
    default:
      fox_unreachable("unknown node");
  }
}

std::string ASTDumper::getDeclNodeName(Decl* decl) const {
  switch (decl->getKind()) {
#define DECL(ID, PARENT) \
  case DeclKind::ID:     \
    return #ID;
#include "Fox/AST/DeclNodes.def"
    default:
      fox_unreachable("unknown node");
  }
}

std::string ASTDumper::getTypeName(Type type) const {
  switch (type->getKind()) {
#define TYPE(ID, PARENT) \
  case TypeKind::ID:     \
    return #ID;
#include "Fox/AST/TypeNodes.def"
    default:
      fox_unreachable("unknown node");
  }
}

std::string ASTDumper::getBasicStmtInfo(Stmt* stmt) const {
  std::ostringstream ss;
  ss << getStmtNodeName(stmt);
  if (isDebug())
    ss << " " << (void*)stmt;
  return ss.str();
}

std::string ASTDumper::getBasicExprInfo(Expr* expr) const {
  std::ostringstream ss;
  ss << getExprNodeName(expr);
  if (isDebug())
    ss << " " << (void*)expr;
  if (auto ty = expr->getType())
    ss << " " << toString(ty);
  return ss.str();
}

std::string ASTDumper::getBasicDeclInfo(Decl* decl) const {
  std::ostringstream ss;
  ss << getDeclNodeName(decl);
  if (isDebug())
    ss << " " << (void*)decl;

  SourceRange range = decl->getRange();
  ss << " " << getSourceLocDump("start", range.getBegin());
  ss << " " << getSourceLocDump("end", range.getEnd());

  return ss.str();
}

std::string ASTDumper::getBasicTypeInfo(Type type) const {
  std::ostringstream ss;
  ss << getTypeName(type);
  if (isDebug())
    ss << " " << (void*)type.getPtr();
  return ss.str();
}

std::string ASTDumper::getBasicValueDeclDump(ValueDecl* decl) const {
  std::ostringstream ss;
  ss << getDeclNodeName(decl);
  if (isDebug())
    ss << " " << (void*)decl;

  ss << " " << getSourceRangeDump("range", decl->getRange()) << " ";

  ss << makeKeyPairDump("id", decl->getIdentifier()->getStr()) << " ";
  ss << getTypeLocDump("type", decl->getTypeLoc(), decl->isConstant()) << " ";

  if (decl->isConstant())
    ss << "const";

  return ss.str();
}

std::string ASTDumper::getOperatorDump(BinaryExpr* expr) const {
  std::ostringstream ss;
  ss << expr->getOpSign() << " (" << expr->getOpName() << ")";
  return ss.str();
}

std::string ASTDumper::getOperatorDump(UnaryExpr* expr) const {
  std::ostringstream ss;
  ss << expr->getOpSign() << " (" << expr->getOpName() << ")";
  return ss.str();
}

std::string ASTDumper::getDeclRecorderDump(DeclContext* dr) const {
  std::ostringstream ss;
  ss << "<DeclContext:" << (void*)dr;
  if (dr->hasParentDeclRecorder())
    ss << ", Parent:" << (void*)dr->getParentDeclRecorder();
  ss << ">";
  return ss.str();
}

std::string ASTDumper::getIdentifierDump(Identifier* id) const {
  return makeKeyPairDump("id", addSingleQuotes(id->getStr()));
}

std::string ASTDumper::getSourceLocDump(string_view label,
                                        SourceLoc sloc) const {
  if (sloc && hasSrcMgr()) {
    std::ostringstream ss;
    CompleteLoc cloc = srcMgr_->getCompleteLoc(sloc);
    ss << "(l" << cloc.line << ",c" << cloc.column << ")";
    return makeKeyPairDump(label, ss.str());
  } 
  return "";
}

std::string ASTDumper::getSourceRangeAsStr(SourceRange range) const {
  if (range && hasSrcMgr()) {
    std::ostringstream ss;
    CompleteLoc begCLoc = srcMgr_->getCompleteLoc(range.getBegin());
    CompleteLoc endCLoc = srcMgr_->getCompleteLoc(range.getEnd());
    if (begCLoc.line != endCLoc.line) {
      ss << "(l" << begCLoc.line << ", c" << begCLoc.column << " to l"
         << endCLoc.line << ", c" << endCLoc.column << ")";
    } else {
      ss << "(l" << begCLoc.line << ", c" << begCLoc.column << " to c"
         << endCLoc.column << ")";
    }
    return ss.str();
  }
  return "";
}

std::string ASTDumper::getSourceRangeDump(string_view label,
                                          SourceRange range) const {
  return makeKeyPairDump(label, getSourceRangeAsStr(range));
}

std::string ASTDumper::getTypeDump(string_view label,
                                   Type ty,
                                   bool isConst) const {
  std::string str = (isConst ? "const " : "") + addSingleQuotes(toString(ty));
  return makeKeyPairDump(label, str);
}

std::string ASTDumper::getTypeLocDump(string_view label,
                                      TypeLoc ty,
                                      bool isConst) const {
  std::ostringstream ss;
  ss << (isConst ? "const ": "") << addSingleQuotes(toString(ty.withoutLoc()));
  if (auto range = ty.getRange())
    ss << " " << getSourceRangeAsStr(range);
  return makeKeyPairDump(label, ss.str());
}

std::string ASTDumper::addDoubleQuotes(string_view str) const {
  std::stringstream ss;
  ss << '"' << str << '"';
  return ss.str();
}

std::string ASTDumper::addSingleQuotes(string_view str) const {
  std::stringstream ss;
  ss << "'" << str << "'";
  return ss.str();
}

void ASTDumper::indent(std::uint8_t num) {
  curIndent_ += num;
}

void ASTDumper::dedent(std::uint8_t num) {
  if (curIndent_) {
    if (curIndent_ >= num)
      curIndent_ -= num;
    else
      curIndent_ = 0;
  }
}

// Dump methods
void Expr::dump() const {
  ASTDumper(std::cerr).dump(const_cast<Expr*>(this));
}

void Stmt::dump() const {
  ASTDumper(std::cerr).dump(const_cast<Stmt*>(this));
}

void Decl::dump() const {
  ASTDumper(std::cerr).dump(const_cast<Decl*>(this));
}

void TypeBase::dump() const {
  std::cerr << this->toDebugString() + '\n';
}

void Type::dump() const {
  if (ty_)
    ty_->dump();
  else
    std::cerr << "Type(nullptr)\n";
}
