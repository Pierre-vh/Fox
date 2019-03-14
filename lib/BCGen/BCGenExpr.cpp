//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : BCGenExpr.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Registers.hpp"
#include "Fox/BCGen/BCGen.hpp"
#include "Fox/VM/InstructionBuilder.hpp"
#include "Fox/AST/Expr.hpp"
#include "Fox/AST/ASTVisitor.hpp"
#include "Fox/AST/ASTWalker.hpp"
#include "Fox/Common/FoxTypes.hpp"

using namespace fox;

//----------------------------------------------------------------------------//
// ExprGenerator
//----------------------------------------------------------------------------//

// The actual class responsible for generating the bytecode of expressions
class BCGen::ExprGenerator : public Generator,
                             private ExprVisitor<ExprGenerator, RegisterValue> {
  using Visitor = ExprVisitor<ExprGenerator, RegisterValue>;
  friend Visitor;
  public:
    ExprGenerator(BCGen& gen, InstructionBuilder& builder, 
                  RegisterAllocator& regAlloc) : Generator(gen, builder),
                  regAlloc(regAlloc) {}

    // Entry point of generation
    void generate(Expr* expr) {
      visit(expr);
    }

    RegisterAllocator& regAlloc;

  private:
    //------------------------------------------------------------------------//
    // "emit" methods 
    // 
    // Theses methods perform some generalized tasks related to bytecode
    // emission
    //------------------------------------------------------------------------//

    // Emit an instruction to store the constant 'val' into the register
    // 'reg'.
    // TODO: Maybe find a better name?
    void emitStoreIntConstant(const RegisterValue& dest, FoxInt val) {
      constexpr auto int16_min = std::numeric_limits<std::int16_t>::min(),
                     int16_max = std::numeric_limits<std::int16_t>::max();
      // Check if the value fits in a int16. In that case, emit a StoreSmallInt
      if ((val >= int16_min) && (val <= int16_max)) {
        builder.createStoreSmallIntInstr(dest.getRegisterNumber(), val);
      }
      // Else, for now, do nothing because I need the constant table to 
      // emit constants large than that.
      fox_unimplemented_feature("Emission & Storage of constants larger than "
        "16 bits");
    }

    //------------------------------------------------------------------------//
    // "visit" methods 
    // 
    // Theses methods will perfom the actual tasks required to emit
    // the bytecode for an Expr.
    //------------------------------------------------------------------------//

    RegisterValue visitBinaryExpr(BinaryExpr*) { 
      // TODO: Numeric binary operations (+ - * / % ** && || < <= > >= ==)
      // TODO: Assignements (will need "AssignExprGenerator")
      
      // For string operations I'll need Strings implemented in the VM.
      // For Char operations I'll need to decide on the handling of chars.
      fox_unimplemented_feature("BinaryExpr Bytecode Generation");
    }

    RegisterValue visitCastExpr(CastExpr*) { 
      // TODO: Numeric casts
      // For the rest we need other things in the VM.
      fox_unimplemented_feature("CastExpr Bytecode Generation");
    }

    RegisterValue visitUnaryExpr(UnaryExpr*) { 
      // TODO
      fox_unimplemented_feature("UnaryExpr Bytecode Generation");
    }

    RegisterValue visitArraySubscriptExpr(ArraySubscriptExpr*) { 
      // Needs Arrays implemented in the VM.
      fox_unimplemented_feature("ArraySubscriptExpr Bytecode Generation");
    }

    RegisterValue visitMemberOfExpr(MemberOfExpr*) { 
      // Unused for now.
      fox_unimplemented_feature("MemberOfExpr Bytecode Generation");
    }

    RegisterValue visitDeclRefExpr(DeclRefExpr*) { 
      // Needs variable code generation, and for global decls, it needs
      // globals implemented in the VM.

      // NOTE: This won't take care of emitting DeclRefs used as
      // the LHS of an assignement. It'll be handled by another
      // visitor.
      fox_unimplemented_feature("DeclRefExpr Bytecode Generation");
    }

    RegisterValue visitCallExpr(CallExpr*) { 
      // Needs functions and calls implemented in the VM.
      fox_unimplemented_feature("CallExpr Bytecode Generation");
    }

    RegisterValue visitCharLiteralExpr(CharLiteralExpr* expr) { 
      // Store the character as an integer in a new register.
      RegisterValue value = regAlloc.allocateNewRegister();
      emitStoreIntConstant(value, expr->getValue());
      return value;
    }

    RegisterValue visitIntegerLiteralExpr(IntegerLiteralExpr* expr) {
      // Store the integer in a new register.
      RegisterValue value = regAlloc.allocateNewRegister();
      emitStoreIntConstant(value, expr->getValue());
      return value;
    }

    RegisterValue visitDoubleLiteralExpr(DoubleLiteralExpr*) { 
      // Needs the constant table since 16 bits floats aren't a thing
      // (= no StoreSmallFloat)
      fox_unimplemented_feature("DoubleLiteralExpr Bytecode Generation");
    }

    RegisterValue visitBoolLiteralExpr(BoolLiteralExpr* expr) { 
      // Store the boolean as an integer in a new register.
      RegisterValue value = regAlloc.allocateNewRegister();
      emitStoreIntConstant(value, expr->getValue());
      return value;
    }

    RegisterValue visitStringLiteralExpr(StringLiteralExpr*) { 
      // Needs strings implemented in the VM
      fox_unimplemented_feature("StringLiteralExpr Bytecode Generation");
    }

    RegisterValue visitArrayLiteralExpr(ArrayLiteralExpr*) {
      // Needs array implemented in the VM
      fox_unimplemented_feature("ArrayLiteralExpr Bytecode Generation");
    }

    // ErrorExprs shouldn't be found in BCGen.
    RegisterValue visitErrorExpr(ErrorExpr*) { 
      fox_unreachable("ErrorExpr found past semantic analysis");
    }

    // UnresolvedDeclRefExprs shouldn't be found in BCGen.
    RegisterValue visitUnresolvedDeclRefExpr(UnresolvedDeclRefExpr*) { 
      fox_unreachable("UnresolvedDeclRefExpr found past semantic analysis");
    }

};

//----------------------------------------------------------------------------//
// BCGen Entrypoints
//----------------------------------------------------------------------------//

void BCGen::emitExpr(InstructionBuilder& builder, Expr* expr) {
  // This is temporarily put here (until work starts on FuncDecl BCGen).
  // It'll be moved to an argument passed to this function after that
  RegisterAllocator regAlloc;

  ExprGenerator(*this, builder, regAlloc).generate(expr);
}