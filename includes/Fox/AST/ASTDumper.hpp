//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : ASTDumper.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file declares the "ASTDumper" class, which is used to 
// print the ast to any ostream.
//
// TODO: Rewrite this class completely.
//----------------------------------------------------------------------------//

#pragma once

#include "ASTVisitor.hpp"
#include <iosfwd>

namespace fox {
  class SourceManager;
  class Type;
  class TypeLoc;
  class ASTDumper : /*private*/ SimpleASTVisitor<ASTDumper, void> {
    using Inherited = SimpleASTVisitor<ASTDumper, void>;
    friend Inherited;
    public:
      /// Creates a new ASTDumper 
      /// \param srcMgr the SourceManager to use to convert SourceLocs into
      ///        human-readable location information.
      /// \param out the output stream
      /// \param baseIndent the base identation of the AST dump.
      ASTDumper(SourceManager& srcMgr, std::ostream& out,
                std::uint16_t baseIndent = 0);

      /// Creates a new ASTDumper that doesn't use a SourceManager.
      /// Dumps will be less precise and won't show the loc information.
      /// \param out the output stream
      /// \param baseIndent the base identation of the AST dump.
      ASTDumper(std::ostream& out, std::uint16_t baseIndent = 0);

      /// Prints a detailed dump of the AST. Mostly for debug use.
      template<typename Ty>
      void dump(Ty&& value) {
        debug_ = true;
        visit(std::forward<Ty>(value));
      }

      /// Prints the AST in a more compact/user-friendly form.
      template<typename Ty>
      void print(Ty&& value) {
        debug_ = false;
        visit(std::forward<Ty>(value));
      }

      /// The output stream
      std::ostream& out;

    private:
      // Expressions
      void visitBinaryExpr(BinaryExpr* node);
      void visitCastExpr(CastExpr* node);
      void visitUnaryExpr(UnaryExpr* node);
      void visitArraySubscriptExpr(ArraySubscriptExpr* node);
      void visitMemberOfExpr(MemberOfExpr* node);
      void visitDeclRefExpr(DeclRefExpr* node);
      void visitUnresolvedDeclRefExpr(UnresolvedDeclRefExpr* node);
      void visitCallExpr(CallExpr* node);

      // Expressions : Literals
      void visitCharLiteralExpr(CharLiteralExpr* node);
      void visitIntegerLiteralExpr(IntegerLiteralExpr* node);
      void visitDoubleLiteralExpr(DoubleLiteralExpr* node);
      void visitBoolLiteralExpr(BoolLiteralExpr* node);
      void visitStringLiteralExpr(StringLiteralExpr* node);
      void visitArrayLiteralExpr(ArrayLiteralExpr* node);
      void visitErrorExpr(ErrorExpr* expr);

      // Stmts
      void visitCompoundStmt(CompoundStmt* node);
      void visitConditionStmt(ConditionStmt* node);
      void visitWhileStmt(WhileStmt* node);
      void visitReturnStmt(ReturnStmt* node);

      // Decls
      void visitUnitDecl(UnitDecl* node);
      void visitVarDecl(VarDecl* node);
      void visitParamDecl(ParamDecl* node);
      void visitFuncDecl(FuncDecl* node);
      void visitBuiltinFuncDecl(BuiltinFuncDecl* node);

      bool isDebug() const;

      std::string toString(Type type) const;
      std::string toString(TypeLoc type) const;
      std::string toString(SourceRange range) const;

      // Returns the name of the file, or the "alternative" string if
      // the file doesn't exist or the SourceManager isn't available.
      string_view getFileNameOr(FileID file, string_view alternative);
      bool hasSrcMgr() const;

      /// Prints the indentation to \out and returns \ref out
      /// \param num additional indentation depth to use
      /// \returns \ref out
      std::ostream& dumpLine(std::uint8_t num = 0);

      /// \param num additional indentation depth to use
      /// \returns the string containing the indentation.
      std::string getIndent(std::uint8_t num = 0) const;

      // Returns the name of the node by using it's pointer
      std::string getStmtNodeName(Stmt* stmt) const;
      std::string getExprNodeName(Expr* expr) const;
      std::string getDeclNodeName(Decl* decl) const;

      // Returns a string containing basic information about a node
      // Its name followed by its address.
      std::string getBasicStmtInfo(Stmt* stmt) const;
      std::string getBasicExprInfo(Expr* expr) const;
      std::string getBasicDeclInfo(Decl* decl) const;
      std::string getValueDeclInfo(ValueDecl* decl) const;

      // Operator dumps
      std::string getOperatorDump(BinaryExpr* expr) const;
      std::string getOperatorDump(UnaryExpr* expr) const;

      /// \returns a formatted string "<DeclContext: address, Parent: address>"
      std::string getDeclCtxtDump(DeclContext* dr) const;
      /// \returns a formatted string: "<label:coords>"
      std::string getSourceLocDump(string_view label, SourceLoc sloc) const;
      /// \returns a formatted string: "<label:beg_line:beg_col-end_line:end_col>"
      std::string getSourceRangeDump(string_view label, SourceRange range) const;

      void indent(std::uint8_t num = 1);
      void dedent(std::uint8_t num = 1);

      SourceManager* srcMgr_ = nullptr;
      std::uint16_t curIndent_ = 0;

      /// debug flag
      bool debug_ = false;
  };
}
