//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : ASTDumper.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the "ASTDumper" class, which is used to 
// print the ast to any ostream.
//----------------------------------------------------------------------------//

#pragma once

#include "ASTVisitor.hpp"
#include <iosfwd>

namespace fox {
  class SourceManager;
  class ASTDumper : /*private*/ SimpleASTVisitor<ASTDumper, void> {
    using Inherited = SimpleASTVisitor<ASTDumper, void>;
    public:
      ASTDumper(SourceManager& srcMgr, std::ostream& out, const uint8_t& offsettabs = 0);
      ASTDumper(std::ostream& out, const uint8_t& offsettabs = 0);

      // Prints the AST as a dump, which will create a highly detailed
      // dump of the AST
      template<typename Ty>
      void dump(Ty&& value) {
        debug_ = true;
        visit(std::forward<Ty>(value));
      }

      // Prints the AST in a more compact, user friendly fashion.
      template<typename Ty>
      void print(Ty&& value) {
        debug_ = false;
        visit(std::forward<Ty>(value));
      }

      // Expressions
      void visitBinaryExpr(BinaryExpr* node);
      void visitCastExpr(CastExpr* node);
      void visitUnaryExpr(UnaryExpr* node);
      void visitArraySubscriptExpr(ArraySubscriptExpr* node);
      void visitMemberOfExpr(MemberOfExpr* node);
      void visitDeclRefExpr(DeclRefExpr* node);
      void visitFunctionCallExpr(FunctionCallExpr* node);

      // Expressions : Literals
      void visitCharLiteralExpr(CharLiteralExpr* node);
      void visitIntegerLiteralExpr(IntegerLiteralExpr* node);
      void visitFloatLiteralExpr(FloatLiteralExpr* node);
      void visitBoolLiteralExpr(BoolLiteralExpr* node);
      void visitStringLiteralExpr(StringLiteralExpr* node);
      void visitArrayLiteralExpr(ArrayLiteralExpr* node);

      // Stmts
      void visitNullStmt(NullStmt* node);
      void visitCompoundStmt(CompoundStmt* node);
      void visitConditionStmt(ConditionStmt* node);
      void visitWhileStmt(WhileStmt* node);
      void visitReturnStmt(ReturnStmt* node);

      // Decls
      void visitUnitDecl(UnitDecl* node);
      void visitVarDecl(VarDecl* node);
      void visitParamDecl(ParamDecl* node);
      void visitFuncDecl(FuncDecl* node);

    private:
      // We need a custom visit method for Type to avoid
      // calling visitXXXType and just use Type->toString()
      void visit(Type type);

      // We also want to use the base class's visit methods
      using Inherited::visit;

      bool isDebug() const;

      std::string toString(Type type) const;

      const SourceManager::SourceData* getSourceData(FileID fid);
      bool hasSrcMgr() const;

      // Prints getOffset() and getIndent() to out_ then returns out_
      // Can add a number as parameter to add a "temporary" indent, just for this line.
      std::ostream& dumpLine(std::uint8_t num = 0);

      // sets offset_ to the correct number of tabs required.
      void recalculateOffset();

      // returns the indent required by curIndent_
      std::string getIndent(const uint8_t& num = 0) const;

      // Returns the name of the node by using it's pointer
      std::string getStmtNodeName(Stmt* stmt) const;
      std::string getExprNodeName(Expr* expr) const;
      std::string getDeclNodeName(Decl* decl) const;
      std::string getTypeName(Type type) const;

      // Returns a string containing basic information about a node : It's name followed by it's adress. Maybe more in the future.
      std::string getBasicStmtInfo(Stmt* stmt) const;
      std::string getBasicExprInfo(Expr* expr) const;
      std::string getBasicDeclInfo(Decl* decl) const;
      std::string getBasicTypeInfo(Type type) const;
      std::string getBasicValueDeclDump(ValueDecl* decl) const;

      // Dump an operator in 2 different ways, depending on dumpOperatorsAsNames_
      std::string getOperatorDump(BinaryExpr* expr) const;
      std::string getOperatorDump(UnaryExpr* expr) const;

      // Returns a formatted string "<DeclContext (adress), Parent: (adress)>"
      std::string getDeclRecorderDump(DeclContext* dr) const;
      // Returns a formatted string, "<ID:(idstring)>"
      std::string getIdentifierDump(Identifier* id) const;
      // Returns a formatted string, "<(label):(coords)>"
      std::string getSourceLocDump(string_view label, SourceLoc sloc) const;
      std::string getSourceRangeDump(string_view label, SourceRange range) const;

      std::string getSourceRangeAsStr(SourceRange range) const;
      // Returns a formatted string  "<(label):'(type)'>
      std::string getTypeDump(string_view label, Type ty, bool isConst = false) const;
      std::string getTypeLocDump(string_view label, TypeLoc ty, bool isConst = false) const;
      // Returns value enclosed with "".
      std::string addDoubleQuotes(string_view str) const;
      // Returns the value enclosed with ''
      std::string addSingleQuotes(string_view str) const;

      // Returns a formatted string "<(label):(value)>
      template<typename TyA,typename TyB>
      std::string makeKeyPairDump(TyA label,TyB value) const {
        std::ostringstream ss;
        ss << "<" << label << ":" << value << ">";
        return ss.str();
      }

      void indent(std::uint8_t num = 1);
      void dedent(std::uint8_t num = 1);

      std::ostream& out_;
      SourceManager* srcMgr_ = nullptr;
      std::string offset_;
      uint16_t curIndent_ = 0, offsetTabs_ = 0;

      // Options
      bool debug_ = false;
  };
}
