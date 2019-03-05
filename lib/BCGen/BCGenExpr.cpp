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

// TODO: Refactor the code here. It's pretty much a prototype, so the code
//       needs to be cleaned up a bit.

// The actual class responsible for generating the bytecode of expression
class BCGen::ExprGenerator : public Generator, private ASTWalker,
                      private ExprVisitor<ExprGenerator, void> {
  public:
    ExprGenerator(BCGen& gen, InstructionBuilder& builder) :
      Generator(gen), builder(builder) {}

    // Entry point
    void generate(Expr* expr) {
      walk(expr);
    }

  private:
    InstructionBuilder& builder;
    // TODO
};


void BCGen::emitExpr(InstructionBuilder& builder, Expr* expr) {
  ExprGenerator(*this, builder).generate(expr);
}