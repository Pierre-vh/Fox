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
                             ExprVisitor<ExprGenerator, RegisterValue> {
  using Visitor = ExprVisitor<ExprGenerator, RegisterValue>;
  friend Visitor;
  public:
    ExprGenerator(BCGen& gen, BCBuilder& builder, 
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

    // If 'reg' can be recycled, recycle it, else, return a new 
    // temporary register.
    RegisterValue tryReuseRegister(RegisterValue& reg) {
      if(reg.canRecycle())
        return regAlloc.recycle(std::move(reg));
      return regAlloc.allocateTemporary();
    }

    // If possible, recycle a live temporary register from the list.
    // (note: this method will always prefer the smallest register numbers)
    // Else, it returns a new temporary register.
    RegisterValue 
    tryReuseRegisters(reference_initializer_list<RegisterValue> regs) {
      RegisterValue* best = nullptr;
      for (auto& reg : regs) {
        if (reg.get().canRecycle()) {
          // Can this become our best candidate?
          if((!best) || (best->getAddress() > reg.get().getAddress())) 
            best = &(reg.get());
        }
      }

      // Recycle the best candidate
      if(best)
        return regAlloc.recycle(std::move(*best));
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
      auto ssi_min = bc_limits::storeSmallInt_min;
      auto ssi_max = bc_limits::storeSmallInt_max;
      // Check if the value can be stored using StoreSmallInt
      if ((val >= ssi_min) && (val <= ssi_max)) {
        builder.createStoreSmallIntInstr(dest.getAddress(), val);
        return;
      }
      // Else, store the constant in the constant table and emit a LoadIntK
      auto kId = bcGen.getConstantID(val);
      builder.createLoadIntKInstr(dest.getAddress(), kId);
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

      // When we have an unary minus, and the child is int, bool or double literal, 
      // directly emit the literal with a negative
      // value instead of generating a NegInt or something.
      if (expr->getOp() == UnOp::Minus) {
        if (auto intLit = dyn_cast<IntegerLiteralExpr>(subExpr))
          return visitIntegerLiteralExpr(intLit,    /*asNegative*/ true);

        if (auto doubleLit = dyn_cast<DoubleLiteralExpr>(subExpr))
          return visitDoubleLiteralExpr(doubleLit,  /*asNegative*/ true);
      }
      // Else compile it normally

      RegisterValue childReg = visit(subExpr);

      // Handle unary plus directly as it's a no-op
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

    RegisterValue visitDeclRefExpr(DeclRefExpr* expr) { 
      ValueDecl* decl = expr->getDecl();
      // Reference to Global declarations
      if(!decl->isLocal())
        fox_unimplemented_feature("Global DeclRefExpr BCGen");
      // Reference to Local Variables
      if(VarDecl* var = dyn_cast<VarDecl>(decl))
        return regAlloc.useVar(var);
      // Reference to Parameter decls
      if(ParamDecl* param = dyn_cast<ParamDecl>(decl))
        fox_unimplemented_feature("ParamDecl DeclRefExpr BCGen");
      fox_unimplemented_feature("Unknown Local Decl Kind");
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

    RegisterValue visitIntegerLiteralExpr(IntegerLiteralExpr* expr, 
                                          bool asNegative = false) {
      RegisterValue dest = regAlloc.allocateTemporary();
      FoxInt value = asNegative ? -expr->getValue() : expr->getValue();
      emitStoreIntConstant(dest, value);
      return dest;
    }

    RegisterValue visitDoubleLiteralExpr(DoubleLiteralExpr* expr, 
                                         bool asNegative = false) { 
      RegisterValue dest = regAlloc.allocateTemporary();
      FoxDouble value = asNegative ? -expr->getValue() : expr->getValue();
      auto kID = bcGen.getConstantID(value);
      builder.createLoadDoubleKInstr(dest.getAddress(), kID);
      return dest;
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

RegisterValue BCGen::genExpr(BCBuilder& builder, 
                             RegisterAllocator& regAlloc, Expr* expr) {
  return ExprGenerator(*this, builder, regAlloc).generate(expr);
}

void BCGen::genDiscardedExpr(BCBuilder& builder, 
                             RegisterAllocator& regAlloc, Expr* expr) {
  ExprGenerator(*this, builder, regAlloc).generate(expr);
}