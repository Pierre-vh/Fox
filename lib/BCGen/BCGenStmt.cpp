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
    using StableInstrConstIter = BCBuilder::StableInstrConstIter;

    // The type used to store jump offsets. Doesn't necessarily
    // match the one of the instructions.
    using jump_offset_t = std::int32_t;

    // The current maximum jump offset possible (positive or negative)
    // is the max (positive or negative) value of a 16 bit signed number:
    // 2^15-1
    static constexpr jump_offset_t max_jump_offset = (1 << 15)-1;
    // Same as max_jump_offset but negative
    static constexpr jump_offset_t min_jump_offset = -max_jump_offset;

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

    /// Represents a 'jump point', a point in the bytecode buffer that we
    /// want to jump to.
    /// This class is also responsible for fixing 'jump' instructions.
    class JumpPoint {
      public:
        /// Creates a JumpPoint to the next instruction after \p instr
        static JumpPoint 
        createForAfterInstr(BCBuilder& builder, StableInstrIter instr) {
          return JumpPoint(builder, Kind::AfterIter, instr);
        }

        /// Creates a JumpPoint 'past-the-end' of the instruction buffer.
        /// Useful for when you want to jump to the next instruction that
        /// will be inserted in the buffer.
        static JumpPoint 
        createForEnd(BCBuilder& builder) {
          // If the Builder is empty, we'll want to jump to the beginning
          // of the instruction buffer
          if(builder.empty())
            return JumpPoint(builder, Kind::BufferBeg);
          // Else we want to just past the last instruction emitted
          return JumpPoint(builder, Kind::AfterIter, 
                           builder.getLastInstrIter());
        }

        /// Fixes a "Jump" instruction \p jump so it jumps to
        /// the JumpPoint
        void fixJumpInstr(StableInstrIter jump) const {
          assert(jump->isAnyJump() && "not a jump!");
          // Calculate the distance + decrement it
          // (because jumps are relative to the next instruction)
          auto rawDistance = distance(jump, getTargetIter())-1;
          // Check if the distance is acceptable
          // TODO: Replace this check by a proper diagnostic
          assert((rawDistance >= min_jump_offset) 
            && (rawDistance <= max_jump_offset)
            && "Jumping too far");
          // Now that we know that the conversion is safe, convert it to a 
          // jump_offset_t
          jump_offset_t offset = rawDistance;
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

        /// The bytecode builder
        BCBuilder& builder;

      private:
        /// The Kind of JumpPoint this is.
        enum class Kind : std::uint8_t {
          /// For when we want to jump to the first instruction
          /// in the bytecode buffer.
          BufferBeg,
          /// For when we want to jump to the instruction after 'iter'
          AfterIter,
        };

        /// Returns an iterator to the target instruction
        ///   For Kind::BufferBeg, returns an iterator to the
        ///     beginning of the buffer.
        ///   For Kind::AfterIter, returns (iter_+1);
        StableInstrConstIter getTargetIter() const {
          switch (kind_) {
            case Kind::BufferBeg:
              return StableInstrConstIter::getBegin(builder.vector);
            case Kind::AfterIter: 
              return ++StableInstrConstIter(iter_);
            default:
              fox_unreachable("unknown JumpPoint::Kind");
          }
        }

        Kind kind_;
        StableInstrConstIter iter_;

        JumpPoint(BCBuilder& builder, Kind kind, 
          StableInstrConstIter iter = StableInstrConstIter())
          : builder(builder), kind_(kind), iter_(iter) { }
    };

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
          JumpPoint::createForEnd(builder).fixJumpInstr(jumpIfFalse);
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
          JumpPoint::createForEnd(builder).fixJumpInstr(jumpIfTrue);
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
          // ...and make jumpIfFalse jump to the next instr that will be emitted
          JumpPoint::createForEnd(builder).fixJumpInstr(jumpIfFalse);
        }
        else {
          // If generated something, complete both jumps:
          //    jumpIfFalse should jump past JumpEnd
          JumpPoint::createForAfterInstr(builder, jumpEnd)
            .fixJumpInstr(jumpIfFalse);
          //    jumpEnd should jump to the last instruction emitted
          JumpPoint::createForEnd(builder).fixJumpInstr(jumpEnd);
        }
      }
    }

    void visitWhileStmt(WhileStmt* stmt) {
      // Create the loop context
      LoopContext loopCtxt(regAlloc);
      // Create a JumpPoint to the beginning of the loop
      auto loopBeg = JumpPoint::createForEnd(builder);
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
      // Fix it
      loopBeg.fixJumpInstr(jumpToBeg);
      // Fix the 'skipBody' jump so it jumps past the 'jumpToBeg'
      JumpPoint::createForAfterInstr(builder, jumpToBeg)
        .fixJumpInstr(skipBodyJump);
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