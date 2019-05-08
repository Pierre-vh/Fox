//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : BCGenExpr.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Registers.hpp"
#include "Fox/AST/ASTContext.hpp"
#include "Fox/BCGen/BCGen.hpp"
#include "Fox/BC/BCBuilder.hpp"
#include "Fox/AST/Expr.hpp"
#include "Fox/AST/Types.hpp"
#include "Fox/AST/ASTVisitor.hpp"
#include "Fox/AST/ASTWalker.hpp"
#include "Fox/AST/Types.hpp"
#include "Fox/Common/FoxTypes.hpp"
#include "Fox/Common/LLVM.hpp"
#include "llvm/ADT/Optional.h"

using namespace fox;

/// Binary Operator Kinds
using BinOp = BinaryExpr::OpKind;
/// Unary Operator Kinds
using UnOp = UnaryExpr::OpKind;

//----------------------------------------------------------------------------//
// AssignementGenerator : Declaration
//----------------------------------------------------------------------------// 

using AGGenFunc = std::function<RegisterValue(RegisterValue)>;

class BCGen::AssignementGenerator : public Generator,
                             ExprVisitor<AssignementGenerator, RegisterValue,
                                                            AGGenFunc, BinOp> {
  using Visitor = ExprVisitor<AssignementGenerator, RegisterValue, 
                                                    AGGenFunc, BinOp>;
  friend Visitor;
  public:  
    AssignementGenerator(BCGen& gen, BCBuilder& builder, 
                         ExprGenerator& exprGen);

    /// The BCGen::ExprGenerator that created this AssignementGenerator 
    ExprGenerator& exprGen;
    /// The RegisterAllocator associted with the exprGenerator
    RegisterAllocator& regAlloc;
  
    /// Generates the bytecode for an assignement expression \p expr
    RegisterValue generate(BinaryExpr* expr);

    /// The AssignementGenerator 'Gen' function. This is a 'thunk'
    /// function that can do many things. What the thunk does
    /// depends on the entry point used.
    /// Its signature is (RegisterValue) -> RegisterValue
    /// The goal is to place the thing we need to assign in the
    /// register passed as parameter and return the parameter.
    /// FIXME: This was used for a previous version of BCGen, but is
    ///        now borderline useless. Maybe remove it?
    using GenFunc = AGGenFunc;

  private:
    RegisterValue
    visitBinaryExpr(BinaryExpr* expr, GenFunc gen, BinOp op);
    RegisterValue
    visitCastExpr(CastExpr* expr, GenFunc gen, BinOp op);
    RegisterValue
    visitUnaryExpr(UnaryExpr* expr, GenFunc gen, BinOp op);
    RegisterValue
    visitArraySubscriptExpr(ArraySubscriptExpr* expr, GenFunc gen, BinOp op);
    RegisterValue
    visitMemberOfExpr(MemberOfExpr* expr, GenFunc gen, BinOp op);
    RegisterValue
    visitDeclRefExpr(DeclRefExpr* expr, GenFunc gen, BinOp op);
    RegisterValue
    visitUnresolvedDeclRefExpr(UnresolvedDeclRefExpr* expr, GenFunc gen, BinOp op);
    RegisterValue
    visitCallExpr(CallExpr* expr, GenFunc gen, BinOp op);
    RegisterValue
    visitAnyLiteralExpr(AnyLiteralExpr* expr, GenFunc gen, BinOp op);
    RegisterValue
    visitErrorExpr(ErrorExpr* expr, GenFunc gen, BinOp op);
};

//----------------------------------------------------------------------------//
// ExprGenerator
//----------------------------------------------------------------------------// 

// The class responsible for generating the bytecode of expressions
class BCGen::ExprGenerator : public Generator,
                             ExprVisitor<ExprGenerator, RegisterValue, 
                                         /*args*/ RegisterValue> {
  using Visitor = ExprVisitor<ExprGenerator, RegisterValue, RegisterValue>;
  friend Visitor;
  public:
    ExprGenerator(BCGen& gen, BCBuilder& builder, 
                  RegisterAllocator& regAlloc) : Generator(gen, builder),
                  regAlloc(regAlloc) {}

    /// Entry point of the ExprGenerator.
    RegisterValue generate(Expr* expr) {
      return visit(expr);
    }  
    
    /// Entry point of the ExprGenerator
    /// \p dest is the destination register for the expression.
    /// The return RegisterValue shall be equal to \p dest
    RegisterValue generate(Expr* expr, RegisterValue reg) {
      return visit(expr, std::move(reg));
    }

    RegisterAllocator& regAlloc;

  private:
    /// Binary Operator Kinds
    using BinOp = BinaryExpr::OpKind;
    /// Unary Operator Kinds
    using UnOp = UnaryExpr::OpKind;
    /// A std::initializer_list of references to objects
    template<typename Ty>
    using reference_initializer_list 
      = std::initializer_list< std::reference_wrapper<Ty> >;

    //------------------------------------------------------------------------//
    // Helper methods
    // 
    // Helper functions performing various tasks. Generalizes/shortens
    // some common patterns used in this generator.
    //------------------------------------------------------------------------//

    /// \returns true if \p expr can be generated using a "XXXInt" instruction
    /// such as AddInt, SubInt, etc.
    /// This is true if both operands are of Int, Bool or Char type.
    bool canGenToIntBinop(BinaryExpr* expr) {
      auto check = [](Expr* expr) {
        Type type = expr->getType();
        assert(type && "type is null");
        return type->isIntType() 
            || type->isBoolType()
            || type->isCharType();
      };
      if (check(expr->getLHS())) {
        assert(check(expr->getRHS()) 
          && "Inconsistent types");
        return true;
      }
      return false;
    }

    /// Chooses a destination register for a function: uses dest if valid,
    /// else allocates a new temporary.
    RegisterValue getDestReg(RegisterValue dest) {
      if(dest) return dest;
      return regAlloc.allocateTemporary();
    }

    /// Chooses a destination register for an expression
    ///  -> Uses \p dest when it's non null
    ///  -> Else, uses the best recyclable RV in \p hints 
    ///     (uses the one with the lowest address possible)
    ///  -> Else allocates a new temporary
    ///
    /// After execution, the hint that has been recycled will be dead
    /// (=evaluate to false) and other hints will be left untouched.
    RegisterValue 
    getDestReg(RegisterValue dest, 
               reference_initializer_list<RegisterValue> hints) {
      if(dest) return dest;
      
      RegisterValue* best = nullptr;
      for (auto& hint : hints) {
        if (hint.get().canRecycle()) {
          // Can this become our best candidate?
          if((!best) || (best->getAddress() > hint.get().getAddress())) 
            best = &(hint.get());
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

    // Emits a call to a builtin function from a list of expressions
    void emitBuiltinCall(BuiltinID bID, RegisterValue dest, ArrayRef<Expr*> args) {
      // Reserve registers
      SmallVector<RegisterValue, 4> callRegs;
      regAlloc.allocateCallRegisters(callRegs, args.size()+1);

      // Put a reference to the builtin in the base register
      regaddr_t baseAddr = callRegs.front().getAddress();
      builder.createLoadBuiltinFuncInstr(baseAddr, bID);

      // Compile the args
      for (std::size_t k = 0, size = args.size(); k < size; ++k)
        callRegs[k+1] = visit(args[k], std::move(callRegs[k+1]));

      // Check what the builtin returns
      // FIXME: Is this a good idea to use the ASTContext for this? What bothers
      //        me the most is that I need to include the whole ASTContext header
      //        for this bit. (+getBuiltinFuncReturnType exists only for this
      //        ATM)
      bool returnsVoid = ctxt.getBuiltinFuncReturnType(bID)->isVoidType();

      // Gen the call
      if(returnsVoid)
        builder.createCallVoidInstr(baseAddr);
      else {
        // Decide on the destination register, if possible reusing the base
        // registers
        // TODO: Try to reuse the arguments registers if possible too
        dest = getDestReg(std::move(dest), {callRegs.front()});
        builder.createCallInstr(baseAddr, dest.getAddress());
      }
    }

    // Emit an instruction to store the constant 'val' into the register
    // 'reg'.
    void emitStoreIntConstant(regaddr_t dest, FoxInt val) {
      auto ssi_min = bc_limits::storeSmallInt_min;
      auto ssi_max = bc_limits::storeSmallInt_max;
      // Check if the value can be stored using StoreSmallInt
      if ((val >= ssi_min) && (val <= ssi_max)) {
        builder.createStoreSmallIntInstr(dest, val);
        return;
      }
      // Else, store the constant in the constant table and emit a LoadIntK
      auto kId = bcGen.getConstantID(val);
      builder.createLoadIntKInstr(dest, kId);
    }

    // Generates the adequate instruction(s) to perform a binary
    // operation on doubles
    void 
    emitDoubleBinOp(BinOp op, regaddr_t dst, regaddr_t lhs, regaddr_t rhs) {
      assert((lhs != rhs) && "lhs and rhs are identical");
      // Emit
      switch (op) {
        case BinOp::Add:  // +
          builder.createAddDoubleInstr(dst, lhs, rhs);
          break;
        case BinOp::Sub:  // -
          builder.createSubDoubleInstr(dst, lhs, rhs);
          break;
        case BinOp::Mul:  // *
          builder.createMulDoubleInstr(dst, lhs, rhs);
          break;
        case BinOp::Div:  // /
          builder.createDivDoubleInstr(dst, lhs, rhs);
          break;
        case BinOp::Mod:  // %
          builder.createModDoubleInstr(dst, lhs, rhs);
          break;
        case BinOp::Pow:  // **
          builder.createPowDoubleInstr(dst, lhs, rhs);
          break;
        case BinOp::LE:   // <=
          builder.createLEDoubleInstr(dst, lhs, rhs);
          break;
        case BinOp::GE:   // >=
          builder.createGEDoubleInstr(dst, lhs, rhs);
          break;
        case BinOp::LT:   // <
          builder.createLTDoubleInstr(dst, lhs, rhs);
          break;
        case BinOp::GT:   // >
          builder.createGTDoubleInstr(dst, lhs, rhs);
          break;
        case BinOp::Eq:   // ==
          builder.createEqDoubleInstr(dst, lhs, rhs);
          break;
        case BinOp::NEq:  // !=
          // != isn't implemented in the vm, it's just implemented
          // as !(a == b). This requires 2 instructions.
          builder.createEqDoubleInstr(dst, lhs, rhs);
          builder.createLNotInstr(dst, dst);
          break;
        case BinOp::LAnd: // &&
        case BinOp::LOr:  // ||
          fox_unreachable("cannot apply these operators on doubles");
        default:
          fox_unreachable("Unhandled binary operation kind");
      }
    }

    // Generates the adequate instruction(s) to perform a binary
    // operation on integers.
    // This is used to generate operations involving chars, 
    // booleans and ints.
    void emitIntBinOp(BinOp op, regaddr_t dst, regaddr_t lhs, regaddr_t rhs) {
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
          break;
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
    RegisterValue genNumericOrBoolBinaryExpr(BinaryExpr* expr, 
                                             RegisterValue dest) {
      assert((expr->getType()->isNumericOrBool()));
      
      // Gen the LHS
      RegisterValue lhsReg = visit(expr->getLHS());
      regaddr_t lhsAddr = lhsReg.getAddress();
      assert(lhsReg && "Generated a dead register for the LHS");

      // Gen the RHS
      RegisterValue rhsReg = visit(expr->getRHS());
      regaddr_t rhsAddr = rhsReg.getAddress();
      assert(rhsReg && "Generated a dead register for the RHS");
      
      // Choose the destination register
      RegisterValue dstReg = getDestReg(std::move(dest), {lhsReg, rhsReg});
      regaddr_t dstAddr = dstReg.getAddress();

      // Dispatch to the appropriate generator function

      // Integer or Boolean expressions
      if (canGenToIntBinop(expr))
        emitIntBinOp(expr->getOp(), dstAddr, lhsAddr, rhsAddr);
      // Double operands
      else if (expr->getLHS()->getType()->isDoubleType()) {
        assert(expr->getRHS()->getType()->isDoubleType()
          && "Inconsistent Operands");
        emitDoubleBinOp(expr->getOp(), dstAddr, lhsAddr, rhsAddr);
      }
      else 
        fox_unreachable("unhandled situation : operands are "
          "neither int, bools or doubles");
      return dstReg;
    }

    RegisterValue genConcatBinaryExpr(BinaryExpr* expr, RegisterValue dest) {
      assert(expr->isConcat() && "not a concatenation");
      //  The return type of this expression should be a string
      assert(expr->getType()->isStringType() && "doesn't return a string");

      Expr* lhs = expr->getLHS();
      Expr* rhs = expr->getRHS();

      // string + string concatenation
      if(lhs->getType()->isStringType() && rhs->getType()->isStringType()) {
        // Generate a call to the concat builtin
        emitBuiltinCall(BuiltinID::strConcat, std::move(dest), {lhs, rhs});
        return dest;
      }

      fox_unimplemented_feature("Unhandled Concatenation Situation");
    }

    //------------------------------------------------------------------------//
    // "visit" methods 
    // 
    // These methods will perfom the actual tasks required to emit
    // the bytecode for an Expr.
    // They take a RegisterValue as argument. It's the destination register
    // but it can be omitted (pass a RegisterValue()), however when it is
    // present EVERY visitXXX method MUST store the result of the expression
    // inside it. This is enforced by an assertion in visit() in debug mode.
    // NOTE: For now it's unused, but it'll be used when generating assignements
    // to optimize them whenever possible.
    //------------------------------------------------------------------------//

    RegisterValue visit(Expr* expr, RegisterValue dest) {
      #ifndef NDEBUG
        // In debug mode, check that the destination is respected
        regaddr_t expectedAddr = dest ? dest.getAddress() : 0;
        RegisterValue resultRV = Visitor::visit(expr, std::move(dest));
        if (dest) {
          assert((expectedAddr == resultRV.getAddress())
          && "A destination register was provided but was not respected");
        }
        return resultRV;
      #else 
        return Visitor::visit(expr, std::move(dest));
      #endif
    }

    RegisterValue visit(Expr* expr) {
      // Directly use Visitor::visit so we bypass the useless checks
      // in visit(Expr*, RegisterValue)
      return Visitor::visit(expr, RegisterValue());
    }

    RegisterValue 
    visitBinaryExpr(BinaryExpr* expr, RegisterValue dest) { 
      assert((expr->getOp() != BinOp::Invalid)
        && "BinaryExpr with OpKind::Invalid past semantic analysis");
      if (expr->isAssignement()) {
        AssignementGenerator assignGen(bcGen, builder, *this);
        RegisterValue reg = assignGen.generate(expr);
        // Copy in the destination register if required
        if (dest && (reg != dest)) {
          builder.createCopyInstr(dest.getAddress(), reg.getAddress());
          return dest;
        }
        // Else just return
        return reg;
      }
      if (expr->isConcat())
        return genConcatBinaryExpr(expr, std::move(dest));
      if (expr->getType()->isNumericOrBool())
        return genNumericOrBoolBinaryExpr(expr, std::move(dest));
      fox_unimplemented_feature("Non-numeric BinaryExpr BCGen");
    }

    RegisterValue 
    visitCastExpr(CastExpr* expr, RegisterValue dest) {
      // Visit the child
      Expr* subExpr = expr->getExpr();
      RegisterValue childReg = visit(subExpr);

      // If this is a useless cast (cast from a type to the same type)
      // just return childReg
      if(expr->isUseless()) return childReg;

      Type ty = expr->getType();
      Type subTy = subExpr->getType();
      regaddr_t childRegAddr = childReg.getAddress();

      RegisterValue dstReg = getDestReg(std::move(dest), {childReg});

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

    RegisterValue 
    visitUnaryExpr(UnaryExpr* expr, RegisterValue dest) { 
      assert((expr->getOp() != UnOp::Invalid)
        && "UnaryExpr with OpKind::Invalid past semantic analysis");

      Expr* subExpr = expr->getExpr();

      // When we have an unary minus, and the child is int, bool or double literal, 
      // directly emit the literal with a negative
      // value instead of generating a NegInt or something.
      if (expr->getOp() == UnOp::Minus) {
        if (auto intLit = dyn_cast<IntegerLiteralExpr>(subExpr))
          return visitIntegerLiteralExpr(intLit, std::move(dest), 
                                            /*asNegative*/ true);

        if (auto doubleLit = dyn_cast<DoubleLiteralExpr>(subExpr))
          return visitDoubleLiteralExpr(doubleLit, std::move(dest), 
                                              /*asNegative*/ true);
      }
      // Else compile it normally

      RegisterValue childReg = visit(subExpr);

      // Handle unary plus directly as it's a no-op
      if(expr->getOp() == UnOp::Plus) return childReg;

      regaddr_t childAddr = childReg.getAddress();

      RegisterValue destReg = getDestReg(std::move(dest), {childReg});
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

    RegisterValue 
    visitArraySubscriptExpr(ArraySubscriptExpr*, RegisterValue) { 
      // Needs Arrays implemented in the VM.
      fox_unimplemented_feature("ArraySubscriptExpr BCGen");
    }

    RegisterValue 
    visitMemberOfExpr(MemberOfExpr*, RegisterValue) { 
      // Unused for now.
      fox_unimplemented_feature("MemberOfExpr BCGen");
    }

    RegisterValue 
    visitDeclRefExpr(DeclRefExpr* expr, RegisterValue dest) { 
      ValueDecl* decl = expr->getDecl();
      // References to Functions
      if (auto func = dyn_cast<FuncDecl>(decl)) {
        auto fID = static_cast<func_id_t>(bcGen.getBCFunction(func).getID());
        dest = getDestReg(std::move(dest));
        builder.createLoadFuncInstr(dest.getAddress(), fID);
        return dest;
      }
      // References to builtins
      if (auto builtin = dyn_cast<BuiltinFuncDecl>(decl)) {
        auto bID = builtin->getBuiltinID();
        dest = getDestReg(std::move(dest));
        builder.createLoadBuiltinFuncInstr(dest.getAddress(), bID);
        return dest;
      }
      assert((isa<VarDecl>(decl) || isa<ParamDecl>(decl))
        && "unknown ValueDecl kind");
      // Reference to Global variables
      if(!decl->isLocal())
        fox_unimplemented_feature("Global DeclRefExpr BCGen");
      // Reference to local decls
      RegisterValue varReg = regAlloc.useDecl(decl);
      if (dest && (dest != varReg)) {
        // If we have a destination register, emit a Copy instr so the result
        // is located in the dest reg (as requested).
        builder.createCopyInstr(dest.getAddress(), varReg.getAddress());
        return dest;
      }
      return varReg;
    }

    RegisterValue
    visitCallExpr(CallExpr* expr, RegisterValue dest) { 
      // The list of expressions to compile, in order.
      SmallVector<Expr*, 8> exprs;
      exprs.reserve(1 + expr->numArgs());
      
      // First, the callee
      exprs.push_back(expr->getCallee());
      // Then the args
      {
        ArrayRef<Expr*> args = expr->getArgs();
        exprs.append(args.begin(), args.end());
      }

      // Reserve the registers
      SmallVector<RegisterValue, 8> regs;
      regs.reserve(exprs.size());
      regAlloc.allocateCallRegisters(regs, exprs.size());

      // Save the 'base' reg addr.
      regaddr_t baseAddr = regs.front().getAddress();

      // Compile the expressions
      assert(exprs.size() == regs.size());
      for (std::size_t k = 0, size = regs.size(); k < size; ++k) {
        // Move the registers out so the function can use them, but store
        // them back after.
        regs[k] = visit(exprs[k], std::move(regs[k]));
      }

      // Use CallVoid for void functions
      if (expr->getType()->isVoidType()) {
        assert(!dest 
          && "CallExpr has void type, but is expected to return a result");
        builder.createCallVoidInstr(baseAddr);
        return RegisterValue();
      }
      // Else just use 'Call'.
      dest = getDestReg(std::move(dest));
      builder.createCallInstr(baseAddr, dest.getAddress());
      return dest;
    }

    RegisterValue 
    visitCharLiteralExpr(CharLiteralExpr* expr, RegisterValue dest) { 
      dest = getDestReg(std::move(dest));
      emitStoreIntConstant(dest.getAddress(), expr->getValue());
      return dest;
    }

    RegisterValue 
    visitIntegerLiteralExpr(IntegerLiteralExpr* expr, RegisterValue dest,
                            bool asNegative = false) {
      dest = getDestReg(std::move(dest));
      FoxInt value = asNegative ? -expr->getValue() : expr->getValue();
      emitStoreIntConstant(dest.getAddress(), value);
      return dest;
    }

    RegisterValue 
    visitDoubleLiteralExpr(DoubleLiteralExpr* expr, RegisterValue dest,
                           bool asNegative = false) { 
      dest = getDestReg(std::move(dest));
      FoxDouble value = asNegative ? -expr->getValue() : expr->getValue();
      auto kID = bcGen.getConstantID(value);
      builder.createLoadDoubleKInstr(dest.getAddress(), kID);
      return dest;
    }

    RegisterValue 
    visitBoolLiteralExpr(BoolLiteralExpr* expr, RegisterValue dest) { 
      dest = getDestReg(std::move(dest));
      emitStoreIntConstant(dest.getAddress(), expr->getValue());
      return dest;
    }

    RegisterValue 
    visitStringLiteralExpr(StringLiteralExpr* expr, RegisterValue dest) { 
      dest = getDestReg(std::move(dest));
      auto string = expr->getValue();
      if (string.size() == 0) // "" literal
        builder.createNewStringInstr(dest.getAddress());
      else                    // Other literals
        builder.createLoadStringKInstr(dest.getAddress(), 
                                       bcGen.getConstantID(string));
      return dest;
    }

    RegisterValue 
    visitArrayLiteralExpr(ArrayLiteralExpr*, RegisterValue) {
      // Needs array implemented in the VM
      fox_unimplemented_feature("ArrayLiteralExpr BCGen");
    }

    // ErrorExprs shouldn't be found in BCGen.
    RegisterValue 
    visitErrorExpr(ErrorExpr*, RegisterValue) { 
      fox_unreachable("ErrorExpr found past semantic analysis");
    }

    // UnresolvedDeclRefExprs shouldn't be found in BCGen.
    RegisterValue 
    visitUnresolvedDeclRefExpr(UnresolvedDeclRefExpr*, RegisterValue) { 
      fox_unreachable("UnresolvedDeclRefExpr found past semantic analysis");
    }

};

//----------------------------------------------------------------------------//
// AssignementGenerator : Implementation
//----------------------------------------------------------------------------// 

BCGen::AssignementGenerator::
AssignementGenerator(BCGen& gen, BCBuilder& builder, ExprGenerator& exprGen)
  : Generator(gen, builder), exprGen(exprGen), regAlloc(exprGen.regAlloc) {}

RegisterValue BCGen::AssignementGenerator::generate(BinaryExpr* expr) {
  assert(expr->isAssignement() && "Not an Assignement");
  // for this entry point, the generator function simply calls the
  // exprGen to generate the RHS into the desired register.
  auto genFunc = [&](RegisterValue dest) {
    return exprGen.generate(expr->getRHS(), std::move(dest));
  };
  return visit(expr->getLHS(), genFunc, expr->getOp());
}

RegisterValue BCGen::AssignementGenerator::
visitBinaryExpr(BinaryExpr*, GenFunc, BinOp) {
  // Shouldn't be possible in LHS of an Assignement 
  //    Reason: BinaryExprs cannot produce LValues
  fox_unreachable("Unhandled Assignement: Cannot Assign to a BinaryExpr");
}

RegisterValue BCGen::AssignementGenerator::
visitCastExpr(CastExpr*, GenFunc, BinOp) {
  // Shouldn't be possible in LHS of an Assignement 
  //    Reason: CastExprs cannot produce LValues
  fox_unreachable("Unhandled Assignement: Cannot Assign to a CastExpr");
}

RegisterValue BCGen::AssignementGenerator::
visitUnaryExpr(UnaryExpr*, GenFunc, BinOp) {
  // Shouldn't be possible in LHS of an Assignement 
  //    Reason: UnaryExprs cannot produce LValues
  fox_unreachable("Unhandled Assignement: Cannot Assign to a UnaryExpr");
}

RegisterValue BCGen::AssignementGenerator::
visitArraySubscriptExpr(ArraySubscriptExpr*, GenFunc, BinOp) {
  // VM doesn't support arrays yet
  fox_unimplemented_feature("ArraySubscript Assignement");
  // -> Gen the Subscripted Expression (SSE) using exprGen
  // -> Gen the Index (IDX) using exprGen
  // -> Gen something like "SetSubscript SSE IDX SRC
}

RegisterValue BCGen::AssignementGenerator::
visitMemberOfExpr(MemberOfExpr*, GenFunc, BinOp) {
  // VM doesn't support objects yet
  fox_unimplemented_feature("MemberOfExpr Assignement");}

RegisterValue BCGen::AssignementGenerator::
visitDeclRefExpr(DeclRefExpr* expr, GenFunc gen, BinOp op) {
  // Assert that the only assignement possible is a vanilla one '='.
  // So, if in the future I add +=, -=, etc. this doesn't fail
  // silently.
  assert((op == BinOp::Assign) && "Unsupported assignement type");
  // Avoid 'unreferenced formal parameter'
  op;
  return gen(regAlloc.useDecl(expr->getDecl()));
}

RegisterValue BCGen::AssignementGenerator::
visitCallExpr(CallExpr*, GenFunc, BinOp) {
  // Shouldn't be possible in LHS of an Assignement 
  //    Reason: CallExprs cannot produce LValues
  fox_unreachable("Unhandled Assignement: Cannot Assign to a CallExpr");
}

RegisterValue BCGen::AssignementGenerator::
visitAnyLiteralExpr(AnyLiteralExpr*, GenFunc, BinOp) {
  // Shouldn't be possible in LHS of an Assignement 
  //    Reason: Literals cannot produce LValues
  fox_unreachable("Unhandled Assignement: "
    "Cannot Assign to a Literal (AnyLiteralExpr)");
}

RegisterValue BCGen::AssignementGenerator::
visitErrorExpr(ErrorExpr*, GenFunc, BinOp) {
  fox_unreachable("ErrorExpr found past Semantic Analysis");
}

RegisterValue BCGen::AssignementGenerator::
visitUnresolvedDeclRefExpr(UnresolvedDeclRefExpr*, GenFunc, BinOp) {
  fox_unreachable("UnresolvedDeclRef found past Semantic Analysis");
}

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
