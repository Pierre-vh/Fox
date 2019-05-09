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
#include "Fox/BC/BCUtils.hpp"
#include "Fox/Common/FoxTypes.hpp"
#include "Fox/Common/string_view.hpp"
#include <memory>
#include <unordered_map>

namespace fox {
  class ASTContext;
  class DiagnosticEngine;
  class BCBuilder;
  class BCFunction;
  class FuncDecl;
  class BCModule;
  class Expr;
  class RegisterAllocator;
  class RegisterValue;

  class BCGen {
    public:
      /// \param ctxt the ASTContext
      /// \param theModule the BCModule in which bytecode will be emitted.
      ///        The BCModule is assumed to be empty. The initial data that
      ///        might be contained inside it will not be read/considered.
      BCGen(ASTContext& ctxt, BCModule& theModule);

      /// Make this class non copyable
      BCGen(const BCGen&) = delete;
      BCGen& operator=(const BCGen&) = delete;

      /// Generates the bytecode of a single unit \p unit
      void genUnit(UnitDecl* unit);

      /// \returns the unique identifier for the string constant \p str
      ///          in \ref theModule 's string constants array.
      constant_id_t getConstantID(string_view strview);

      /// \returns the unique identifier for the int constant \p value
      ///          in \ref theModule 's int constants array.
      constant_id_t getConstantID(FoxInt value);

      /// \returns the unique identifier for the double constant \p value
      ///          in \ref theModule 's double constants array.
      constant_id_t getConstantID(FoxDouble value);

      ASTContext& ctxt;
      DiagnosticEngine& diagEngine;
      BCModule& theModule;

    private:
      // Generates (emits) the bytecode for a GLOBAL VarDecl "var" 
      void genGlobalVar(BCBuilder& builder, VarDecl* var);

      // Generates (emits) the bytecode for a function declaration "func" 
      void genFunc(FuncDecl* func);

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

      /// \returns the BCFunction object for a FuncDecl*
      BCFunction& getBCFunction(FuncDecl* func);

      class Generator;
      class ExprGenerator;
      class AssignementGenerator;
      class LocalDeclGenerator;
      class StmtGenerator;

      // The map of functions to BCFunction&
      std::unordered_map<FuncDecl*, BCFunction&> funcs_;

      // Constant maps, used to 'unique' constants.
      // Note: For the string constants map, we store the hash of the string
      // instead of the string itself to save some space.
      std::unordered_map<std::size_t, constant_id_t> strConstsMap_;
      std::unordered_map<FoxInt, constant_id_t>      intConstsMap_;
      std::unordered_map<FoxDouble, constant_id_t>   doubleConstsMap_;
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