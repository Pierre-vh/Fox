//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : BCGenExpr.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/BCGen/BCGen.hpp"
#include "Fox/VM/InstructionBuilder.hpp"
#include "Fox/AST/Expr.hpp"
#include "Fox/AST/ASTVisitor.hpp"
#include "Fox/AST/ASTWalker.hpp"

using namespace fox;

//----------------------------------------------------------------------------//
// ExprGenerator
//----------------------------------------------------------------------------//

// The actual class responsible for generating the bytecode of expressions
class BCGen::ExprGenerator : public Generator,
                             private ExprVisitor<ExprGenerator, void> {
  using Visitor = ExprVisitor<ExprGenerator, void>;
  friend Visitor;
  public:
    ExprGenerator(BCGen& gen, InstructionBuilder& builder) :
      Generator(gen, builder) {}

    // Entry point of generation
    void generate(Expr* expr) {
      visit(expr);
    }

  private:
    //----------------------------------------------------------------------//
    // "visit" methods 
    // 
    // Theses methods will perfom the actual tasks required to emit
    // the bytecode for a node.
    //----------------------------------------------------------------------//

    // Expressions
    void visitBinaryExpr(BinaryExpr*) { /*TODO*/ }
    void visitCastExpr(CastExpr*) { /*TODO*/ }
    void visitUnaryExpr(UnaryExpr*) { /*TODO*/ }
    void visitArraySubscriptExpr(ArraySubscriptExpr*) { /*TODO*/ }
    void visitMemberOfExpr(MemberOfExpr*) { /*TODO*/ }
    void visitDeclRefExpr(DeclRefExpr*) { /*TODO*/ }
    void visitUnresolvedDeclRefExpr(UnresolvedDeclRefExpr*) { /*TODO*/ }
    void visitCallExpr(CallExpr*) { /*TODO*/ }

    // Expressions : Literals
    void visitCharLiteralExpr(CharLiteralExpr*) { /*TODO*/ }
    void visitIntegerLiteralExpr(IntegerLiteralExpr*) { /*TODO*/ }
    void visitDoubleLiteralExpr(DoubleLiteralExpr*) { /*TODO*/ }
    void visitBoolLiteralExpr(BoolLiteralExpr*) { /*TODO*/ }
    void visitStringLiteralExpr(StringLiteralExpr*) { /*TODO*/ }
    void visitArrayLiteralExpr(ArrayLiteralExpr*) { /*TODO*/ }
    void visitErrorExpr(ErrorExpr*) { /*TODO*/ }
};

//----------------------------------------------------------------------------//
// BCGen Entrypoints
//----------------------------------------------------------------------------//

void BCGen::emitExpr(InstructionBuilder& builder, Expr* expr) {
  ExprGenerator(*this, builder).generate(expr);
}