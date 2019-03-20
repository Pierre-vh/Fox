//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : BCGen.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the interface to code generation for Fox, which
// converts a Fox AST into bytecode.
//----------------------------------------------------------------------------//

#pragma once

#include "Fox/AST/ASTFwdDecl.hpp"
#include <memory>

namespace fox {
  class ASTContext;
  class DiagnosticEngine;
  class BCModuleBuilder;
  class BCModule;
  class Expr;
  class BCGen {
    public:
      BCGen(ASTContext& ctxt);

      // Performs codegen on a single unit and returns the
      // resulting BCModule.
      std::unique_ptr<BCModule> genUnit(UnitDecl* unit);

      ASTContext& ctxt;
      DiagnosticEngine& diagEngine;

    private:
      // Generates (emits) the bytecode for a GLOBAL VarDecl "var" 
      // using the builder "builder".
      void genGlobalVar(BCModuleBuilder& builder, VarDecl* var);

      // Generates (emits) the bytecode for a function declaration "func" 
      // using the builder "builder".
      void genFunc(BCModuleBuilder& builder, FuncDecl* func);

      // Generates (emits) the bytecode for a statement "stmt" using the 
      // builder "builder".
      void genStmt(BCModuleBuilder& builder, Stmt* stmt);

      // Generates (emits) the bytecode for an expression "expr" using the 
      // builder "builder".
      void genExpr(BCModuleBuilder& builder, Expr* expr);

      class Generator;
      class ExprGenerator;
      class StmtGenerator;

  };

  // Common base class for every "generator".
  class BCGen::Generator {
    public:
      BCGen& bcGen;
      DiagnosticEngine& diagEngine;
      ASTContext& ctxt;
      BCModuleBuilder& builder;

    protected:
      Generator(BCGen& bcGen, BCModuleBuilder& builder) : 
        bcGen(bcGen), builder(builder),
        diagEngine(bcGen.diagEngine), ctxt(bcGen.ctxt) {}
  };
}