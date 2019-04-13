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
  class BCBuilder;
  class BCModule;
  class Expr;
  class RegisterAllocator;
  class RegisterValue;

  class BCGen {
    public:
      BCGen(ASTContext& ctxt);

      // Performs codegen on a single unit and returns the
      // resulting BCModule.
      std::unique_ptr<BCModule> genUnit(UnitDecl* unit);

      ASTContext& ctxt;
      DiagnosticEngine& diagEngine;

    private:
      /// the maximum number of functions that can be contained
      /// in a single module.
      static constexpr std::size_t max_functions = 0xFFFF;

      // Generates (emits) the bytecode for a GLOBAL VarDecl "var" 
      void genGlobalVar(BCBuilder& builder, VarDecl* var);

      // Generates (emits) the bytecode for a function declaration "func" 
      void genFunc(BCModule& bcmodule, FuncDecl* func);

      // Generates (emits) the bytecode for a statement "stmt"
      void genStmt(BCBuilder& builder, 
                   RegisterAllocator& regAlloc, Stmt* stmt);

      // Generates (emits) the bytecode for an expression "expr".
      // Returns the RegisterValue managing the register containing the
      // result of the expr.
      RegisterValue genExpr(BCBuilder& builder, 
                   RegisterAllocator& regAlloc, Expr* expr);

      // Generates (emits) the bytecode for an expression "expr", but
      // immediately discards the result.
      void genDiscardedExpr(BCBuilder& builder, 
                            RegisterAllocator& regAlloc, Expr* expr);

      // Generates (emits) the bytecode for a local declaration "decl"
      void genLocalDecl(BCBuilder& builder, 
                        RegisterAllocator& regAlloc, Decl* decl);

      class Generator;
      class ExprGenerator;
      class LocalDeclGenerator;
      class StmtGenerator;

  };

  // Common base class for every "generator".
  class BCGen::Generator {
    public:
      BCGen& bcGen;
      DiagnosticEngine& diagEngine;
      ASTContext& ctxt;
      BCBuilder& builder;

    protected:
      Generator(BCGen& bcGen, BCBuilder& builder) : 
        bcGen(bcGen), builder(builder),
        diagEngine(bcGen.diagEngine), ctxt(bcGen.ctxt) {}
  };
}