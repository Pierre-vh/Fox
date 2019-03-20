//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : BCGenExpr.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Registers.hpp"
#include "Fox/BCGen/BCGen.hpp"
#include "Fox/BC/BCBuilder.hpp"
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
    ExprGenerator(BCGen& gen, BCModuleBuilder& builder, 
                  RegisterAllocator& regAlloc) : Generator(gen, builder),
                  regAlloc(regAlloc) {}

    // Entry point of generation
    RegisterValue generate(Expr* expr) {
      return visit(expr);
    }

    RegisterAllocator& regAlloc;

  private:
    using BinOp = BinaryExpr::OpKind;
    using UnOp = UnaryExpr::OpKind;
    template<typename Ty>
    using reference_initializer_list = std::initializer_list<std::reference_wrapper<Ty> >;

    //------------------------------------------------------------------------//
    // Helper methods
    // 
    // Helper functions performing various tasks. Generalizes/shortens
    // some common patterns used in this generator.
    //------------------------------------------------------------------------//

    // Returns true if the type is an integer or a boolean
    bool isIntOrBool(Type type) {
      return (type->isIntType() || type->isBoolType());
    }

    // Returns true if this binary expression's operand are
    // both integers or booleans.
    bool areOperandsIntOrBools(BinaryExpr* expr) {
      if (isIntOrBool(expr->getLHS()->getType())) {
        assert(isIntOrBool(expr->getRHS()->getType()) && "Inconsistent types");
        return true;
      }
      return false;
    }

    // If 'reg' is a live temporary register, returns std::move(reg).
    // Else, returns a new allocated register.
    RegisterValue tryReuseRegister(RegisterValue& reg) {
      if(reg.isTemporary())
        return std::move(reg);
      return regAlloc.allocateTemporary();
    }

    // If possible, reuses a live temporary register from the list. 
    // In that case, the chosen register is moved and returned.
    // 
    // Else, returns a new temporary register.
    RegisterValue 
    tryReuseRegisters(reference_initializer_list<RegisterValue> regs) {
      RegisterValue* best = nullptr;
      for (auto& reg : regs) {
        // Only reuse alive temporary registers.
        if (reg.get().isTemporary()) {
          // Can this become our best candidate?
          if((!best) || (best->getAddress() > reg.get().getAddress())) 
            best = &(reg.get());
        }
      }

      // Reuse the best candidate
      if(best)
        return std::move(*best);
      return regAlloc.allocateTemporary();
    }

    //------------------------------------------------------------------------//
    // "emit" methods 
    // 
    // These methods perform some generalized tasks related to bytecode
    // emission
    //------------------------------------------------------------------------//

    // Emit an instruction to store the constant 'val' into the register
    // 'reg'.
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

    // Generates the adequate instruction(s) to perform a binary
    // operation on integers or boolean operands.
    void emitIntegerOrBoolBinaryOp(BinOp op, regaddr_t dst, 
                                  regaddr_t lhs, regaddr_t rhs) {
      assert((lhs != rhs) && "lhs and rhs are identical");
      // Emit
      switch (op) {
        case BinOp::Add:  // +
          builder.createAddIntInstr(dst, lhs, rhs);
          break;
        case BinOp::Sub:  // -
          builder.createSubIntInstr(dst, lhs, rhs);
          break;
        case BinOp::Mul:  // *
          builder.createMulIntInstr(dst, lhs, rhs);
          break;
        case BinOp::Div:  // /
          builder.createDivIntInstr(dst, lhs, rhs);
          break;
        case BinOp::Mod:  // %
          builder.createModIntInstr(dst, lhs, rhs);
          break;
        case BinOp::Pow:  // **
          builder.createPowIntInstr(dst, lhs, rhs);
          break;
        case BinOp::LE:   // <=
          builder.createLEIntInstr(dst, lhs, rhs);
          break;
        case BinOp::GE:   // >=
          // For >=, it's not implemented in the VM, but
          // (a >= b) is the same as (b <= a)
          builder.createLEIntInstr(dst, rhs, lhs);
        case BinOp::LT:   // <
          builder.createLTIntInstr(dst, lhs, rhs);
          break;
        case BinOp::GT:   // >
          // > isn't implemented in the VM too, but
          // (a > b) is the same as !(a <= b). This requires 2 instructions.
          // dest = lhs <= rhs
          builder.createLEIntInstr(dst, lhs, rhs);
          // dest != dest
          builder.createLNotInstr(dst, dst);
          break;
        case BinOp::Eq:   // ==
          builder.createEqIntInstr(dst, lhs, rhs);
          break;
        case BinOp::NEq:  // !=
          // != isn't implemented in the vm, it's just implemented
          // as !(a == b). This requires 2 instructions.
          builder.createEqIntInstr(dst, lhs, rhs);
          builder.createLNotInstr(dst, dst);
          break;
        case BinOp::LAnd: // &&
          builder.createLAndInstr(dst, lhs, rhs);
          break;
        case BinOp::LOr:  // ||
          builder.createLOrInstr(dst, lhs, rhs);
          break;
        default:
          fox_unreachable("Unhandled binary operation kind");
      }
    }

    // Generates the code for a BinaryExpr whose type is a Numeric or
    // Boolean Binary Expr.
    RegisterValue genNumericOrBoolBinaryExpr(BinaryExpr* expr) {
      assert((expr->getType()->isNumericOrBool()));
      assert((expr->getLHS()->getType()->isNumericOrBool())
          && (expr->getRHS()->getType()->isNumericOrBool()));
      
      // Gen the LHS
      RegisterValue lhsReg = visit(expr->getLHS());
      regaddr_t lhsAddr = lhsReg.getAddress();
      assert(lhsReg && "Generated a dead register for the LHS");

      // Gen the RHS
      RegisterValue rhsReg = visit(expr->getRHS());
      regaddr_t rhsAddr = rhsReg.getAddress();
      assert(rhsReg && "Generated a dead register for the RHS");
      
      // Decide on which register to use for the destination, maybe reusing
      // the lhs or rhs.
      RegisterValue dstReg = tryReuseRegisters({lhsReg, rhsReg});
      regaddr_t dstAddr = dstReg.getAddress();

      // Dispatch to the appropriate generator function
      Type lhsType = expr->getLHS()->getType();
      // Double operands
      if (lhsType->isDoubleType()) {
        assert(expr->getRHS()->getType()->isDoubleType()
          && "Inconsistent Operands");
        // TODO
        fox_unimplemented_feature("BCGen of BinaryExprs with Double operands");
      }
      // Integer or Boolean expressions
      else if (isIntOrBool(lhsType)) {
        assert(isIntOrBool(expr->getRHS()->getType())
          && "Inconsistent Operands");
        emitIntegerOrBoolBinaryOp(expr->getOp(), dstAddr, lhsAddr, rhsAddr);
      }
      else 
        fox_unreachable("unhandled situation : operands are "
          "neither int, bools or doubles");
      return dstReg;
    }

    //------------------------------------------------------------------------//
    // "visit" methods 
    // 
    // Theses methods will perfom the actual tasks required to emit
    // the bytecode for an Expr.
    //------------------------------------------------------------------------//

    RegisterValue visitBinaryExpr(BinaryExpr* expr) { 
      assert((expr->getOp() != BinOp::Invalid)
        && "BinaryExpr with OpKind::Invalid past semantic analysis");
      if(expr->isAssignement())
        fox_unimplemented_feature("Assignement BinaryExpr BCGen");
      if (expr->getType()->isNumericOrBool())
        return genNumericOrBoolBinaryExpr(expr);
      fox_unimplemented_feature("Non-numeric BinaryExpr BCGen");
    }

    RegisterValue visitCastExpr(CastExpr* expr) {
      // Visit the child
      Expr* subExpr = expr->getExpr();
      RegisterValue childReg = visit(subExpr);

      // If this is a useless cast (cast from a type to the same type)
      // just return childReg
      if(expr->isUseless()) return childReg;

      Type ty = expr->getType();
      Type subTy = subExpr->getType();
      regaddr_t childRegAddr = childReg.getAddress();

      RegisterValue dstReg = tryReuseRegister(childReg);

      assert(dstReg && "no destination register selected");
      regaddr_t dstRegAddr = dstReg.getAddress();

      // Casts to numeric types
      if (ty->isNumeric()) {
        // Numeric -> Numeric
        if (subTy->isNumeric()) {
          // We know it's a non-useless cast from a numeric type
          // to a different numeric type.
          if (ty->isDoubleType()) {
            assert(subTy->isIntType());
            // It's a Int -> Double cast
            builder.createIntToDoubleInstr(dstRegAddr, childRegAddr);
          }
          else if (ty->isIntType()) {
            // It's a Double -> Int cast
            builder.createDoubleToIntInstr(dstRegAddr, childRegAddr);
          }
          else 
            fox_unreachable("Unhandled numeric type kind");
        }
        // Numeric -> ?
        else {
          fox_unreachable("Unhandled BCGen situation "
            "(CastExpr from non-numeric to numeric");
        }
      }
      // Other casts
      else {
        fox_unimplemented_feature("Non-numeric CastExpr BCGen");
      }

      return dstReg;
    }

    RegisterValue visitUnaryExpr(UnaryExpr* expr) { 
      assert((expr->getOp() != UnOp::Invalid)
        && "UnaryExpr with OpKind::Invalid past semantic analysis");

      Expr* subExpr = expr->getExpr();

      RegisterValue childReg = visit(subExpr);

      // No-op
      if(expr->getOp() == UnOp::Plus) return childReg;

      regaddr_t childAddr = childReg.getAddress();

      RegisterValue destReg = tryReuseRegister(childReg);
      regaddr_t destAddr = destReg.getAddress();

      // Unary LNot '!' is always applied on booleans, so we
      // compile it to a LNot in every scenario.
      if(expr->getOp() == UnOp::LNot)
        builder.createLNotInstr(destAddr, childAddr);

      // Unary Minus '-' is always applied on numeric types, and
      // the child's type should be the same numeric kind
      // as the expr's.
      else if (expr->getOp() == UnOp::Minus) {
        Type ty = expr->getType();
        assert(ty->isNumeric() && "Unary Minus on non-numeric types");
        // Decide what to emit based on the type of the UnaryExpr.
        if(ty->isIntType())
          builder.createNegIntInstr(destAddr, childAddr);
        else if(ty->isDoubleType()) 
          builder.createNegDoubleInstr(destAddr, childAddr);
        else fox_unreachable("Unknown numeric type kind");
      }
      else fox_unreachable("Unknown Unary Operator");

      return destReg;
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

void BCGen::genExpr(BCModuleBuilder& builder, Expr* expr) {
  // This is temporarily put here (until work starts on FuncDecl BCGen).
  // It'll be moved to an argument passed to this function after that
  RegisterAllocator regAlloc;

  ExprGenerator(*this, builder, regAlloc).generate(expr);
}