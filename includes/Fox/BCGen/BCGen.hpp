//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : BCGen.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the interface to code generation for Fox, which
// converts a Fox AST into bytecode.
//----------------------------------------------------------------------------//

namespace fox {
  class ASTContext;
  class DiagnosticEngine;
  class InstructionBuilder;
  class Expr;
  class BCGen {
    public:
      BCGen(ASTContext& ctxt);

      // Generates (emits) the bytecode for an expression "expr" using the 
      // builder "builder".
      void emitExpr(InstructionBuilder& builder, Expr* expr);

      DiagnosticEngine& getDiagnosticEngine() const;

      ASTContext& getASTContext() const;

    private:
      class Generator;
      class ExprGenerator;

      ASTContext& ctxt_;
  };

  // Common base class for every "generator".
  class BCGen::Generator {
    BCGen& bcGen_;
    DiagnosticEngine& diags_;
    ASTContext& ctxt_;
    InstructionBuilder& builder_;
    protected:
      Generator(BCGen& bcGen,  InstructionBuilder& builder) : 
        bcGen_(bcGen), builder_(builder),
        diags_(bcGen_.getDiagnosticEngine()), ctxt_(bcGen_.getASTContext()) {}

      ASTContext& getCtxt() { return ctxt_; }
      DiagnosticEngine& getDiags() { return diags_; }
      BCGen& getBCGen() { return bcGen_; }
      InstructionBuilder& getBuilder() { return builder_; }
  };
}