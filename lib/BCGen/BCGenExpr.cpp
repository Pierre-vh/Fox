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
#include "Fox/AST/Types.hpp"
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
    using BinOp = BinaryExpr::OpKind;

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
        builder.createStoreSmallIntInstr(dest.getAddress(), val);
        return;
      }
      // Else, for now, do nothing because I need the constant table to 
      // emit constants large than that.
      fox_unimplemented_feature("Emission & Storage of constants larger than "
        "16 bits");
    }

    // Generates the adequate instruction to perform a given binary
    // operation on 'lhs' and 'rhs' (registers containing doubles), putting
    // the result in 'rhs'
    /*
    void genBinaryOperationOnDoubles(BinOp op, const RegisterValue& dest, 
      RegisterValue lhs, RegisterValue rhs) {
      // TODO
    }
    */

    // Generates the adequate instruction to perform a given binary
    // operation on 'lhs' and 'rhs' (registers containing ints), putting
    // the result in 'rhs'
    void genBinaryOperationOnInts(BinOp op, const RegisterValue& dest, 
      RegisterValue lhs, RegisterValue rhs) {
      // Fetch the addresses
      regnum_t destAddr = dest.getAddress();
      regnum_t lhsAddr = lhs.getAddress();
      regnum_t rhsAddr = rhs.getAddress();
      // Emit
      switch (op) {
        case BinOp::Add:  // +
          builder.createAddIntInstr(destAddr, lhsAddr, rhsAddr);
          break;
        case BinOp::Sub:  // -
          builder.createSubIntInstr(destAddr, lhsAddr, rhsAddr);
          break;
        case BinOp::Mul:  // *
          builder.createMulIntInstr(destAddr, lhsAddr, rhsAddr);
          break;
        case BinOp::Div:  // /
          builder.createDivIntInstr(destAddr, lhsAddr, rhsAddr);
          break;
        case BinOp::Mod:  // %
          builder.createModIntInstr(destAddr, lhsAddr, rhsAddr);
          break;
        case BinOp::Pow:  // **
          builder.createPowIntInstr(destAddr, lhsAddr, rhsAddr);
          break;
        case BinOp::LE:   // <=
          builder.createLEIntInstr(destAddr, lhsAddr, rhsAddr);
          break;
        case BinOp::GE:   // >=
          // For >=, it's not implemented in the VM, but
          // (a >= b) is the same as (b <= a)
          builder.createLEIntInstr(destAddr, rhsAddr, lhsAddr);
        case BinOp::LT:   // <
          builder.createLTIntInstr(destAddr, lhsAddr, rhsAddr);
          break;
        case BinOp::GT:   // >
          // > isn't implemented in the VM too, but
          // (a > b) is the same as !(a <= b). This requires 2 instructions.
          // dest = lhs <= rhs
          builder.createLEIntInstr(destAddr, lhsAddr, rhsAddr);
          // dest != dest
          builder.createLNotInstr(destAddr, destAddr);
          break;
        case BinOp::Eq:   // ==
          builder.createEqIntInstr(destAddr, lhsAddr, rhsAddr);
          break;
        case BinOp::NEq:  // !=
          // != isn't implemented in the vm, it's just implemented
          // as !(a == b). This requires 2 instructions.
          builder.createEqIntInstr(destAddr, lhsAddr, rhsAddr);
          builder.createLNotInstr(destAddr, destAddr);
          break;
        default:
          fox_unreachable("Unhandled binary operation kind");
      }
    }

    RegisterValue genNumericBinaryExpr(BinaryExpr* expr) {
      assert((expr->getType()->isNumeric()) && "expr is not numeric");
      
      // Gen the LHS
      RegisterValue lhsReg;
      {
        Expr* lhsExpr = expr->getLHS();
        assert(lhsExpr->getType()->isNumeric() 
          && "BinaryExpr is numeric but the LHS isn't!");
        lhsReg = visit(lhsExpr);
        assert(lhsReg.isAlive() && "Generated a dead register for the LHS");
      }

      // Gen the RHS
      RegisterValue rhsReg;
      {
        Expr* rhsExpr = expr->getRHS();
        assert(rhsExpr->getType()->isNumeric() 
          && "BinaryExpr is numeric but the RHS isn't!");
        rhsReg = visit(rhsExpr);
        assert(rhsReg.isAlive() && "Generated a dead register for the RHS");
      }

      // Gen the Op
      // NOTE: I currently allocate a new register for the destination operand,
      // but if one of the lhs/rhs is a temporary and not a variable, 
      // I can use it and return it.
      // For that, add a RegisterValue::isTemporary method and select
      // the LHS/RHS depending on if they're temporaries or not. If both are,
      // select the smaller one
      RegisterValue destReg = regAlloc.allocateTemporary();
      if (expr->getType()->isIntType()) {
        genBinaryOperationOnInts(expr->getOp(), destReg, 
                                 std::move(lhsReg), std::move(rhsReg));
      } 
      else if (expr->getType()->isDoubleType()) {
        // TODO
        fox_unimplemented_feature("Floating-point BinaryExpr BCGen");
        /*genBinaryOperationOnDoubles(expr->getOp(), destReg, 
                                 std::move(lhsReg), std::move(rhsReg));*/
      }
      else 
        fox_unreachable("Unknown Numeric Type Kind");
      return destReg;
    }

    //------------------------------------------------------------------------//
    // "visit" methods 
    // 
    // Theses methods will perfom the actual tasks required to emit
    // the bytecode for an Expr.
    //------------------------------------------------------------------------//

    RegisterValue visitBinaryExpr(BinaryExpr* expr) { 
      if(expr->isAssignement())
        fox_unimplemented_feature("Assignement BinaryExpr BCGen");
      if (expr->getType()->isNumeric())
        return genNumericBinaryExpr(expr);
      fox_unimplemented_feature("Non-numeric BinaryExpr BCGen");
    }

    RegisterValue visitCastExpr(CastExpr*) { 
      // TODO: Numeric casts
      // For the rest we need other things in the VM.
      fox_unimplemented_feature("CastExpr BCGen");
    }

    RegisterValue visitUnaryExpr(UnaryExpr*) { 
      // TODO
      fox_unimplemented_feature("UnaryExpr BCGen");
    }

    RegisterValue visitArraySubscriptExpr(ArraySubscriptExpr*) { 
      // Needs Arrays implemented in the VM.
      fox_unimplemented_feature("ArraySubscriptExpr BCGen");
    }

    RegisterValue visitMemberOfExpr(MemberOfExpr*) { 
      // Unused for now.
      fox_unimplemented_feature("MemberOfExpr BCGen");
    }

    RegisterValue visitDeclRefExpr(DeclRefExpr*) { 
      // Needs variable code generation, and for global decls, it needs
      // globals implemented in the VM.

      // NOTE: This won't take care of emitting DeclRefs used as
      // the LHS of an assignement. It'll be handled by another
      // visitor.
      fox_unimplemented_feature("DeclRefExpr BCGen");
    }

    RegisterValue visitCallExpr(CallExpr*) { 
      // Needs functions and calls implemented in the VM.
      fox_unimplemented_feature("CallExpr BCGen");
    }

    RegisterValue visitCharLiteralExpr(CharLiteralExpr* expr) { 
      // Store the character as an integer in a new register.
      RegisterValue value = regAlloc.allocateTemporary();
      emitStoreIntConstant(value, expr->getValue());
      return value;
    }

    RegisterValue visitIntegerLiteralExpr(IntegerLiteralExpr* expr) {
      // Store the integer in a new register.
      RegisterValue value = regAlloc.allocateTemporary();
      emitStoreIntConstant(value, expr->getValue());
      return value;
    }

    RegisterValue visitDoubleLiteralExpr(DoubleLiteralExpr*) { 
      // Needs the constant table since 16 bits floats aren't a thing
      // (= no StoreSmallFloat)
      fox_unimplemented_feature("DoubleLiteralExpr BCGen");
    }

    RegisterValue visitBoolLiteralExpr(BoolLiteralExpr* expr) { 
      // Store the boolean as an integer in a new register.
      RegisterValue value = regAlloc.allocateTemporary();
      emitStoreIntConstant(value, expr->getValue());
      return value;
    }

    RegisterValue visitStringLiteralExpr(StringLiteralExpr*) { 
      // Needs strings implemented in the VM
      fox_unimplemented_feature("StringLiteralExpr BCGen");
    }

    RegisterValue visitArrayLiteralExpr(ArrayLiteralExpr*) {
      // Needs array implemented in the VM
      fox_unimplemented_feature("ArrayLiteralExpr BCGen");
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