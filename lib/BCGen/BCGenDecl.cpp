//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : BCGenDecl.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/BCGen/BCGen.hpp"
#include "Registers.hpp"
#include "Fox/AST/Decl.hpp"
#include "Fox/AST/Expr.hpp"
#include "Fox/AST/Stmt.hpp"
#include "Fox/AST/TypeVisitor.hpp"
#include "Fox/AST/ASTWalker.hpp"
#include "Fox/AST/ASTVisitor.hpp"
#include "Fox/BC/BCBuilder.hpp"
#include "Fox/BC/BCModule.hpp"
#include "Fox/BC/BCUtils.hpp"
#include "Fox/Common/Errors.hpp"

using namespace fox;

//----------------------------------------------------------------------------//
// DefaultInitGenerator
//
// Generates code to default-init a variable of some type in a given register.
//----------------------------------------------------------------------------//

namespace {
  class DefaultInitGenerator : TypeVisitor<DefaultInitGenerator, void, regaddr_t> {
    using Inherited = TypeVisitor<DefaultInitGenerator, void, regaddr_t>;
    friend Inherited;
    public:
      DefaultInitGenerator(BCBuilder& builder) : builder(builder) {}

      void gen(Type type, regaddr_t dest) {
        visit(type, dest);
      }

      BCBuilder& builder;

    private:
      void visitIntegerType(IntegerType*, regaddr_t dest) {
        builder.createStoreSmallIntInstr(dest, 0);
      }

      void visitDoubleType(DoubleType*, regaddr_t dest) {
        builder.createStoreSmallIntInstr(dest, 0);
      }

      void visitBoolType(BoolType*, regaddr_t dest) {
        builder.createStoreSmallIntInstr(dest, 0);
      }

      void visitCharType(CharType*, regaddr_t dest) {
        builder.createStoreSmallIntInstr(dest, 0);
      }

      void visitStringType(StringType*, regaddr_t dest) {
        builder.createNewStringInstr(dest);
      }

      void visitVoidType(VoidType*, regaddr_t) {
        fox_unreachable("VoidType shouldn't appear in a variable's type");
      }

      void visitErrorType(ErrorType*, regaddr_t) {
        fox_unreachable("ErrorType found past Semantic Analysis");
      }

      void visitFunctionType(FunctionType*, regaddr_t) {
        fox_unreachable("FunctionType shouldn't appear in a variable's type");
      }

      void visitArrayType(ArrayType*, regaddr_t) {
        fox_unimplemented_feature("ArrayType variables default initialization");
      }

      void visitLValueType(LValueType*, regaddr_t) {
        fox_unreachable("LValueTypes shouldn't appear in a variable's type");
      }

      void visitTypeVariableType(TypeVariableType*, regaddr_t) {
        fox_unreachable("TypeVariableType found past Semantic Analysis");
      }

  };
}

//----------------------------------------------------------------------------//
// DeclGenerator 
//----------------------------------------------------------------------------//

class BCGen::LocalDeclGenerator : public Generator,
                                  DeclVisitor<LocalDeclGenerator, void> {
  using Inherited = DeclVisitor<LocalDeclGenerator, void>;
  friend Inherited;
  public:
    LocalDeclGenerator(BCGen& gen, BCBuilder& builder,
                  RegisterAllocator& regAlloc): 
      Generator(gen, builder), regAlloc(regAlloc) {}

    void generate(Decl* decl) {
      visit(decl);
    }

    RegisterAllocator& regAlloc;

  private:
    void visitUnitDecl(UnitDecl*) {
      return fox_unreachable("UnitDecl found at the local level");
    }

    void visitVarDecl(VarDecl* decl) {
      Expr* init = decl->getInitExpr();

      // If the variable doesn't have an initializer, simply default-init it
      if (!init) {
        RegisterValue varReg = regAlloc.initVar(decl);
        DefaultInitGenerator dig(builder);
        dig.gen(decl->getTypeLoc().getType(), varReg.getAddress());
        return;
      }

      // Generate the initializer
      RegisterValue initReg = bcGen.genExpr(builder, regAlloc, init);

      // If possible, store the variable directly in initReg.
      // This should be the most common case.
      if (initReg.canRecycle()) {
        regAlloc.initVar(decl, &initReg); // discard the RegisterValue directly
        assert(!initReg.isAlive() && "hint not consumed");
        return;
      } 

      // Initialize the variable, duplicating the register containing
      // the initializer in the var's designated register.
      RegisterValue varReg = regAlloc.initVar(decl);
      builder.createCopyInstr(varReg.getAddress(), initReg.getAddress());
    }

    void visitParamDecl(ParamDecl*) {
      fox_unimplemented_feature("ParamDecl BCGen");
    }

    void visitFuncDecl(FuncDecl*) {
      return fox_unreachable("FuncDecl found at the local level");
    }

    void visitBuiltinFuncDecl(BuiltinFuncDecl*) {
      return fox_unreachable("BuiltinFuncDecl shouldn't be BCGen'd");
    }
};

//----------------------------------------------------------------------------//
// FuncGenPrologue 
//
// This performs some tasks that are needed in order to correctly generate
// the bytecode for the body of a FuncDecl. One such task is notifying the
// RegisterAllocator of every local variable declaration/usage so it can
// know the number of uses a variable has (to free its register after its last
// use)
//----------------------------------------------------------------------------//

namespace {
  class FuncGenPrologue : ASTWalker, SimpleASTVisitor<FuncGenPrologue, void> {
    using Inherited = SimpleASTVisitor<FuncGenPrologue, void>;
    friend Inherited;
    public:
      FuncGenPrologue(RegisterAllocator& regAlloc) : regAlloc(regAlloc) {}

      void doPrologue(FuncDecl* decl) {
        // Walk the body
        walk(decl->getBody());
      }

      RegisterAllocator& regAlloc;

    private:
      virtual bool handleDeclPre(Decl* decl) override {
        visit(decl);
        // We want to visit the children
        return true;
      }

      virtual std::pair<Expr*, bool> handleExprPre(Expr* expr) override {
        visit(expr);
        // We want to continue the walk and visit the children
        return {expr, true};
      }

      void visitExpr(Expr*) {
        // no-op
      }

      void visitFuncDecl(FuncDecl*) {
        // Fox does not currently allow functions in a local scope.
        fox_unreachable("FuncDecl found inside a FuncDecl");
      }

      void visitUnitDecl(UnitDecl*) {
        fox_unreachable("UnitDecl found inside a FuncDecl");
      }

      void visitParamDecl(ParamDecl*) {
        fox_unreachable("ParamDecl found inside a FuncDecl");
      }

      void visitBuiltinFuncDecl(BuiltinFuncDecl*) {
        return fox_unreachable("BuiltinFuncDecl shouldn't be BCGen'd");
      }

      void visitVarDecl(VarDecl* decl) {
        assert(decl->isLocal() && "Non-Local VarDecl found in "
          "FuncGenPrologue?");
        regAlloc.addUsage(decl);
      }

      void visitDeclRefExpr(DeclRefExpr* expr) {
        ValueDecl* decl = expr->getDecl();
        assert(decl && "decl is null in DeclRefExpr");
        if (VarDecl* var = dyn_cast<VarDecl>(decl)) { 
          // Local VarDecl
          if(var->isLocal()) 
            regAlloc.addUsage(var);
          // Global VarDecl
          else 
            fox_unimplemented_feature("FuncGenPrologue for DeclRefExpr "
              "of non-local VarDecls");
        }
        else if (ParamDecl* param = dyn_cast<ParamDecl>(decl))
          regAlloc.addUsage(param);
        // else, ignore.
      }

      void visitUnresolvedDeclRefExpr(UnresolvedDeclRefExpr*) {
        fox_unreachable("UnresolvedDeclRefExpr found past semantic analysis");
      }
  };
}

//----------------------------------------------------------------------------//
// BCGen Entrypoints
//----------------------------------------------------------------------------//

void BCGen::genFunc(FuncDecl* func) {
  assert(func && "func is null");
  // Get the (maybe null) parameter list
  ParamList* params = func->getParams();
  // Create the RegisterAllocator for this Function
  RegisterAllocator regAlloc(params);

  // Do the prologue so classes like the RegisterAllocator
  // can be given enough information to correctly generate the bytecode.
  FuncGenPrologue(regAlloc).doPrologue(func);

  // Fetch the BCFunction
  BCFunction& fn = getBCFunction(func);

  // Create the builder
  BCBuilder builder = fn.createBCBuilder();
  // Gen the body.
  genStmt(builder, regAlloc, func->getBody());

  // Check if the last instruction inserted was indeed a Ret instr.
  // If it wasn't, or if the function is empty, insert a RetVoid
  if (builder.empty() || (!builder.getLastInstrIter()->isAnyRet()))
    builder.createRetVoidInstr();
}

void BCGen::genGlobalVar(BCBuilder&, VarDecl*) {  
  // assert(var && var->isGlobal());
  fox_unimplemented_feature("BCGen::genGlobalVar");
}

void BCGen::genLocalDecl(BCBuilder& builder,
                         RegisterAllocator& regAlloc, Decl* decl) {
  assert(decl->isLocal() && "Decl isn't local!");
  LocalDeclGenerator(*this, builder, regAlloc).generate(decl);
}

BCFunction& BCGen::getBCFunction(FuncDecl* func) {
  {
    auto it = funcs_.find(func);
    if(it != funcs_.end())
      return it->second;
  }
  // a BCFunction for this FuncDecl* was not created yet so create it
  assert((theModule.numFunctions() <= bc_limits::max_functions)
    && "Cannot create function: too many functions in the module");
  BCFunction& fn = theModule.createFunction();
  funcs_.insert({func, fn});
  return fn;
}

void BCGen::genUnit(UnitDecl* unit) {
  assert(unit && "arg is nullptr");
  for (Decl* decl : unit->getDecls()) {
    if (FuncDecl* fn = dyn_cast<FuncDecl>(decl))
      genFunc(fn);
    else if (VarDecl* var = dyn_cast<VarDecl>(decl)) 
      fox_unimplemented_feature("Global Var BCGen");
    else 
      fox_unreachable("unknown top level decl kind");
  }
}