//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : BCGenStmt.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/BCGen/BCGen.hpp"
#include "Registers.hpp"
#include "Fox/AST/Stmt.hpp"
#include "Fox/AST/ASTVisitor.hpp"
#include "Fox/BC/BCBuilder.hpp"
#include "Fox/Common/Errors.hpp"

using namespace fox;

//----------------------------------------------------------------------------//
// StmtGenerator 
//----------------------------------------------------------------------------//

class BCGen::StmtGenerator : public Generator,
                             StmtVisitor<StmtGenerator, void> {
  using Visitor = StmtVisitor<StmtGenerator, void>;
  friend Visitor;
  public:
    StmtGenerator(BCGen& gen, BCModuleBuilder& builder,
                  RegisterAllocator& regAlloc): 
      Generator(gen, builder), regAlloc(regAlloc), 
      theModule(builder.getModule()) {}

    void generate(Stmt* stmt) {
      visit(stmt);
    }

    RegisterAllocator& regAlloc;
    BCModule& theModule;

  private:
    // The type used to store jump offsets. Doesn't necessarily
    // match the one of the instructions.
    using jump_offset_t = std::int32_t;

    // The current maximum jump offset possible (positive or negative)
    // is the max (positive or negative) value of a 16 bit signed number:
    // 2^15-1
    static constexpr jump_offset_t max_jump_offset = (1 << 15)-1;

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

    // This calculates the offset needed to jump from 'a' to 'b' so
    // the instruction after 'b' will be the next instruction to be executed.
    jump_offset_t calculateJumpOffset(BCModule::instr_iterator a,
                                      BCModule::instr_iterator b) {
      if(a == b) 
        return 0;

      bool isNegative = false;
      if (b < a) {
        std::swap(a, b);
        isNegative = true;
      }

      // Calculate the distance between the a and the b iterator
      std::size_t diff = distance(a, b);

      // TODO: Replace this assertion by a proper 'fatal' diagnostic explaining
      // the problem. Maybe pass a lambda 'onOutOfRange' as parameter to
      // this function and call onOutOfRange() + return 0; when we try to
      // jump too far.
      assert((diff <= static_cast<std::size_t>(max_jump_offset))
        && "Jump is out of range!");

      // Convert it to jump_offset_t now that we know that it's safe.
      auto offset = static_cast<jump_offset_t>(diff);

      assert((offset != 0) && "offset cannot be zero");

      // Return, applying the minus if needed
      return isNegative ? (-offset) : offset;
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
      RegisterValue condReg = bcGen.genExpr(builder, regAlloc, stmt->getCond());
      regaddr_t regAddr = condReg.getAddress();

      // Create a conditional jump (so we can jump to the else's code if the
      // condition is false)
      auto jumpIfFalse = builder.createJumpIfNotInstr(regAddr, 0);

      // Free the register of the condition
      condReg.free();

      // Gen the 'then'
      visit(stmt->getThen());

      // Check if the "then" emitted any instruction,
      bool isThenEmpty = builder.isLastInstr(jumpIfFalse);

      // Compile the 'else' if present
      if (Stmt* elseBody = stmt->getElse()) {
        // The then is empty
        if(isThenEmpty) {
          // If the then is empty, remove jumpIfFalse and replace it with a JumpIf. 
          // It will be completed later.
          builder.truncate_instrs(jumpIfFalse);
          auto jumpIfTrue = builder.createJumpIfInstr(regAddr, 0);
          // Gen the 'else'
          visit(elseBody);
          // Check if we have generated something.
          if (builder.isLastInstr(jumpIfTrue)) {
            // If the else was empty too, remove everything, including jumpIfTrue, so
            // just the condition's code is left.
            builder.truncate_instrs(jumpIfTrue);
          }
          else {
            // Adjust the jump if we generated something
            auto off = calculateJumpOffset(jumpIfTrue, theModule.instrs_last());
            jumpIfTrue->JumpIf.offset = off;
          }
        }
        // The then is not empty
        else {
          // Create a jump to the end of the condition so the then's code
          // skips the else's code.
          auto jumpEnd = builder.createJumpInstr(0);

          // Gen the 'else'
          visit(elseBody);
          // Check if we have generated something.
          if (builder.isLastInstr(jumpEnd)) {
            // If we generated nothing, remove everything from jumpEnd
            builder.truncate_instrs(jumpEnd);
            // And make jumpIfFalse jump after the last instruction emitteD.
            jumpIfFalse->JumpIfNot.offset =
              calculateJumpOffset(jumpIfFalse, theModule.instrs_last());
          }
          else {
            // If we did generate something, complete both jumps.
            //    jumpIfFalse must execute the else, so jump after jumpEnd
            jumpIfFalse->JumpIfNot.offset = 
              calculateJumpOffset(jumpIfFalse, jumpEnd);
            //    jumpEnd must skip the else, so jump after the last instruction
            //    emitted.
            jumpEnd->Jump.offset = 
              calculateJumpOffset(jumpEnd, theModule.instrs_last());
          }
        }
      }
      // No 'else' statement
      else {
        // If the 'then' was empty too, remove all of the code we've generated
        // related to the then/else, so only the condition's code is left.
        if (isThenEmpty) 
          builder.truncate_instrs(jumpIfFalse);
        // Else, complete 'jumpToElse' to jump after the last instr emitted.
        else 
          jumpIfFalse->JumpIfNot.offset = 
            calculateJumpOffset(jumpIfFalse, theModule.instrs_last());
      }
    }

    void visitWhileStmt(WhileStmt*) {
      fox_unimplemented_feature("WhileStmt BCGen");
      // Don't forget about variable liveliness: I'll need a
      // RegisterAllocator::LoopContext or something like that.
    }

    void visitReturnStmt(ReturnStmt*) {
      fox_unimplemented_feature("ReturnStmt BCGen");
    }
};

//----------------------------------------------------------------------------//
// BCGen Entrypoints
//----------------------------------------------------------------------------//

void BCGen::genStmt(BCModuleBuilder& builder, 
                    RegisterAllocator& regAlloc, 
                    Stmt* stmt) {
  assert(stmt && "stmt is null");
  StmtGenerator(*this, builder, regAlloc).generate(stmt);
}