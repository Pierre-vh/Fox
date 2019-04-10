//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : BCGenStmt.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/BCGen/BCGen.hpp"
#include "Fox/AST/ASTVisitor.hpp"
#include "Fox/AST/Stmt.hpp"
#include "Fox/BC/BCBuilder.hpp"
#include "Fox/Common/Errors.hpp"
#include "LoopContext.hpp"
#include "Registers.hpp"

using namespace fox;

//----------------------------------------------------------------------------//
// StmtGenerator 
//----------------------------------------------------------------------------//

class BCGen::StmtGenerator : public Generator,
                             StmtVisitor<StmtGenerator, void> {
  using Visitor = StmtVisitor<StmtGenerator, void>;
  friend Visitor;
  public:
    StmtGenerator(BCGen& gen, BCBuilder& builder,
                  RegisterAllocator& regAlloc): 
      Generator(gen, builder), regAlloc(regAlloc) {}

    void generate(Stmt* stmt) {
      visit(stmt);
    }

    RegisterAllocator& regAlloc;

  private:
    using StableInstrIter = BCBuilder::StableInstrIter;

    // The type used to store jump offsets. Doesn't necessarily
    // match the one of the instructions.
    using jump_offset_t = std::int32_t;

    // The current maximum jump offset possible (positive or negative)
    // is the max (positive or negative) value of a 16 bit signed number:
    // 2^15-1
    static constexpr std::size_t max_jump_offset = (1 << 15)-1;

    //------------------------------------------------------------------------//
    // "emit" and "gen" methods 
    // 
    // These methods perform some generalized tasks related to bytecode
    // emission
    //------------------------------------------------------------------------//

    void genNode(ASTNode node) {
      if(Decl* decl = node.dyn_cast<Decl*>())
        bcGen.genLocalDecl(builder, regAlloc, decl);
      else if(Expr* expr = node.dyn_cast<Expr*>()) 
        bcGen.genDiscardedExpr(builder, regAlloc, expr);
      else if(Stmt* stmt = node.dyn_cast<Stmt*>()) 
        visit(stmt);
      else 
        fox_unreachable("Unknown ASTNode kind");
    }

    /// Fix a jump \p jump so it jumps to the instruction AFTER \p target.
    /// \param jump The jump to adjust. Must be a jump of some kind.
    /// \param target The last instruction that should be skipped by the jump,
    ///               so '++jump' is the next instruction that will be executed.
    void fixJump(StableInstrIter jump, StableInstrIter target) {
      assert(jump->isAnyJump() && "not a jump!");
      assert((jump != target) && "useless jump");
      std::size_t absoluteDistance;
      bool isBackward = false;
      // Calculate the absolute distance
      {
        auto start = jump, end = target;
        // Check if we try to jump backward. If we do, swap
        // start and end because the distance function expects that
        // its first argument is smaller than the second.
        if (start > end) {
          std::swap(start, end);
          isBackward = true;
        }
        absoluteDistance = distance(start, end);
      }
      // Check if the distance is acceptable
      // TODO: Replace this assertion by a proper 'fatal' diagnostic explaining
      // the problem. Maybe pass a lambda 'onOutOfRange' as parameter to
      // this function and call onOutOfRange() + return 0; when we try to
      // jump too far.
      assert((absoluteDistance <= max_jump_offset) && "Jump is too large!");
      // Now that we know that the conversion is safe, convert the absolute 
      // distance to jump_offset_t
      jump_offset_t offset = absoluteDistance;
      // Reapply the minus sign if needed
      // Note: for backwards jump we need to add an additional offset of 1
      // because jumps are relative to the next instruction.
      if(isBackward) offset = -offset-1;
      // Fix the Jump
      switch (jump->opcode) {
        case Opcode::Jump:
          jump->Jump.offset = offset;
          break;
        case Opcode::JumpIf:
          jump->JumpIf.offset = offset;
          break;
        case Opcode::JumpIfNot:
          jump->JumpIfNot.offset = offset;
          break;
        default:
          fox_unreachable("Unknown Jump Kind!");
      }
    }

    //------------------------------------------------------------------------//
    // "visit" methods 
    // 
    // Theses methods will perfom the actual tasks required to emit
    // the bytecode for a statement.
    //------------------------------------------------------------------------//

    void visitCompoundStmt(CompoundStmt* stmt) {
      // Just visit all the nodes
      for (ASTNode node : stmt->getNodes()) {
        genNode(node);
        if (Stmt* nodeAsStmt = node.dyn_cast<Stmt*>()) {
          // If this is a ReturnStmt, stop here so we don't emit
          // the code after it (since it's unreachable anyway).
          if(isa<ReturnStmt>(nodeAsStmt)) return;
        }
      }
    }

    void visitConditionStmt(ConditionStmt* stmt) {
      // Gen the condition and save its address
      // The RegisterValue is intentionally discarded so it is immediately freed
      regaddr_t regAddr = 
        bcGen.genExpr(builder, regAlloc, stmt->getCond()).getAddress();

      // Create a "JumpIfNot" so we can jump to the else's code
      // when the instruction is false.
      auto jumpIfFalse = builder.createJumpIfNotInstr(regAddr, 0);

      // Gen the 'then'
      visitCompoundStmt(stmt->getThen());

      // Check if the "then" emitted any instruction,
      bool isThenEmpty = builder.isLastInstr(jumpIfFalse);

      // Conditions without elses
      if(!stmt->hasElse()) {
        // If the 'then' was empty, remove all of the code we've generated
        // related to the then/else, so only the condition's code is left.
        if (isThenEmpty) 
          builder.truncate_instrs(jumpIfFalse);
        // Else, complete 'jumpToElse' to jump after the last instr emitted.
        else 
          fixJump(jumpIfFalse, builder.getLastInstrIter());
        return;
      }

      Stmt* elseBody = stmt->getElse();

      // We have a else, and the then was empty
      if(isThenEmpty) {
        // If the then is empty, remove JumpIfNot and replace it with a JumpIf
        builder.truncate_instrs(jumpIfFalse);
        auto jumpIfTrue = builder.createJumpIfInstr(regAddr, 0);
        // Gen the 'else'
        visit(elseBody);
        // Check if we have generated something
        if (builder.isLastInstr(jumpIfTrue)) {
          // If we didn't: Remove everything, including jumpIfTrue, so just the
          // condition's code is left
          builder.truncate_instrs(jumpIfTrue);
        }
        else {
          // If we did: Fix the jump
          fixJump(jumpIfTrue, builder.getLastInstrIter());
        }
      }
      // We have a else, and the then was not empty.
      else {
        // Create a jump to the end of the condition so the then's code
        // skips the else's code.
        auto jumpEnd = builder.createJumpInstr(0);

        // Gen the 'else'
        visit(elseBody);

        // Check if we have generated something.
        if (builder.isLastInstr(jumpEnd)) {
          // If we generated nothing, remove everything including jumpEnd...
          builder.truncate_instrs(jumpEnd);
          // ...and make jumpIfFalse jump after the last instruction emitted.
          fixJump(jumpIfFalse, builder.getLastInstrIter());
        }
        else {
          // If generated something, complete both jumps:
          //    jumpIfFalse should jump past JumpEnd
          fixJump(jumpIfFalse, jumpEnd);
          //    jumpEnd should jump to the last instruction emitted
          fixJump(jumpEnd, builder.getLastInstrIter());
        }
      }
    }

    void visitWhileStmt(WhileStmt* stmt) {
      // Create the loop context
      LoopContext loopCtxt(regAlloc);
      // Save an iterator to the beginning of the loop
      // This is actually an iterator to the last instruction emitted,
      // but as fixJump jumps *after* an instruction, we can use
      // that to jump to the first instruction of the loop.
      auto loopBeg = builder.getLastInstrIter();
      // Compile the condition and save its address.
      // The resulting RegisterValue is intentionally discarded
      // so its register is freed directly.
      auto condAddr = 
        bcGen.genExpr(builder, regAlloc, stmt->getCond()).getAddress();
      // When the condition is false, we skip the body, so create a 
      // JumpIfNot. It'll be completed later
      auto skipBodyJump = builder.createJumpIfNotInstr(condAddr, 0);
      // Gen the body of the loop
      bcGen.genStmt(builder, regAlloc, stmt->getBody());
      // Gen the jump to the beginning of the loop
      auto jumpToBeg = builder.createJumpInstr(0);
      // TODO: Instead of fixing the jump, build it directly using a 
      // "calculateOffSetForJump" function
      fixJump(jumpToBeg, loopBeg);
      // Fix the 'skipBody' jump so it jumps past the 'jumpToBeg'
      fixJump(skipBodyJump, jumpToBeg);
    }

    void visitReturnStmt(ReturnStmt*) {
      fox_unimplemented_feature("ReturnStmt BCGen");
    }
};

//----------------------------------------------------------------------------//
// BCGen Entrypoints
//----------------------------------------------------------------------------//

void BCGen::genStmt(BCBuilder& builder, 
                    RegisterAllocator& regAlloc, 
                    Stmt* stmt) {
  assert(stmt && "stmt is null");
  StmtGenerator(*this, builder, regAlloc).generate(stmt);
}