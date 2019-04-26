//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : VM.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/VM/VM.hpp"
#include "Fox/BC/Instruction.hpp"
#include "Fox/BC/BCModule.hpp"
#include "Fox/BC/BCFunction.hpp"
#include "Fox/Common/Errors.hpp"
#include <cmath>
#include <iterator>

using namespace fox;

VM::VM(BCModule& theModule) : bcModule(theModule) {
  /// The base register will simply be the first register in the
  /// stack.
  baseReg_ = regStack_.data();
}

VM::reg_t* VM::call(BCFunction& func, MutableArrayRef<reg_t> args) {
  assert((func.numParams() == args.size())
    && "Incorrect number of arguments for the function");
  /// Copy the args into registers r0 -> rN
  if(args.size()) {
    regaddr_t k = 0;
    for (auto arg : args)
      setReg(k++, arg);
  }
  /// Run the bytecode
  reg_t* rtr = run(func.getInstructions());
  /// Copy the args back if needed
  if(args.size()) {
    for (regaddr_t k = 0; k < args.size(); k++)
      args[k] = getReg(k);
  }
  return rtr;
}

VM::reg_t* VM::run(ArrayRef<Instruction> instrs) {
  setupIterators(instrs);
  Instruction instr;
  do {
    // Fetch the current instruction
    instr = *curInstr_;
    // Macros used to implement repetitive operations
    #define TRIVIAL_TAC_BINOP_IMPL(ID, TYPE, OP)\
      setReg(instr.ID.dest,\
      getReg<TYPE>(instr.ID.lhs) OP getReg<TYPE>(instr.ID.rhs))
    switch (instr.opcode) {
      case Opcode::NoOp: 
        // NoOp: no-op: do nothing.
        continue;
      case Opcode::StoreSmallInt:
        // StoreSmallInt dest value: Stores value in dest (value: int16)
        setReg(instr.StoreSmallInt.dest, instr.StoreSmallInt.value);
        continue;
      case Opcode::AddInt: 
        // AddInt dest lhs rhs: dest = lhs + rhs (FoxInts)
        TRIVIAL_TAC_BINOP_IMPL(AddInt, FoxInt, +);
        continue;
      case Opcode::AddDouble:
        // AddDouble dest lhs rhs: dest = lhs + rhs (FoxDoubles)
        TRIVIAL_TAC_BINOP_IMPL(AddDouble, FoxDouble, +);
        continue;
      case Opcode::SubInt:
        // SubInt dest lhs rhs: dest = lhs - rhs (FoxInts)
        TRIVIAL_TAC_BINOP_IMPL(SubInt, FoxInt, -);
        continue;
      case Opcode::SubDouble:
        // SubDouble dest lhs rhs: dest = lhs - rhs (FoxDoubles)
        TRIVIAL_TAC_BINOP_IMPL(SubDouble, FoxDouble, -);
        continue;
      case Opcode::MulInt:
        // DivInt dest lhs rhs: dest = lhs * rhs (FoxInts)
        TRIVIAL_TAC_BINOP_IMPL(MulInt, FoxInt, *);
        continue;
      case Opcode::MulDouble:
        // SubDouble dest lhs rhs: dest = lhs * rhs (FoxDoubles)
        TRIVIAL_TAC_BINOP_IMPL(MulDouble, FoxDouble, *);
        continue;
      case Opcode::DivInt:
        // DivInt dest lhs rhs: dest = lhs / rhs (FoxInts)
        // TO-DO: Handle division by zero with something else than an assert
        assert(getReg<FoxInt>(instr.DivInt.rhs) && "division by zero");
        TRIVIAL_TAC_BINOP_IMPL(DivInt, FoxInt, /);
        continue;
      case Opcode::DivDouble:
        // SubDouble dest lhs rhs: dest = lhs / rhs (FoxDoubles)
        // TO-DO: Handle division by zero with something else than an assert
        assert(getReg<FoxDouble>(instr.DivDouble.rhs) && "division by zero");
        TRIVIAL_TAC_BINOP_IMPL(DivDouble, FoxDouble, /);
        continue;
      case Opcode::ModInt:
        // ModInt dest lhs rhs: dest = lhs % rhs (FoxInts)
        // TO-DO: Handle modulo by zero with something else than an assert
        assert(getReg<FoxInt>(instr.ModInt.rhs) && "Modulo by zero");
        TRIVIAL_TAC_BINOP_IMPL(ModInt, FoxInt, %);
        continue;
      case Opcode::ModDouble:
        // ModDouble dest lhs rhs: dest = lhs % rhs (FoxDoubles)
        // TO-DO: Handle modulo by zero with something else than an assert
        assert(getReg<FoxDouble>(instr.ModDouble.rhs) && "Modulo by zero");
        setReg(instr.ModDouble.dest, static_cast<FoxDouble>(
          std::fmod(
            getReg<FoxDouble>(instr.ModDouble.lhs), 
            getReg<FoxDouble>(instr.ModDouble.rhs)
          )
        ));
        continue;
      case Opcode::PowInt:
        // PowInt dest lhs rhs: dest = pow(lhs, rhs) (FoxInts)
        setReg(instr.PowInt.dest, static_cast<FoxInt>(
          std::pow(
            getReg<FoxInt>(instr.PowInt.lhs), 
            getReg<FoxInt>(instr.PowInt.rhs)
          )
        ));
        continue;
      case Opcode::PowDouble:
        // PowDouble dest lhs rhs: dest = pow(lhs, rhs) (FoxDoubles)
        setReg(instr.PowDouble.dest, static_cast<FoxDouble>(
          std::pow(
            getReg<FoxDouble>(instr.PowDouble.lhs), 
            getReg<FoxDouble>(instr.PowDouble.rhs)
          )
        ));
        continue;
      case Opcode::NegInt:
        // NegInt dest src : dest = -src (FoxInts)
        setReg(instr.NegInt.dest, -getReg<FoxInt>(instr.NegInt.src));
        continue;
      case Opcode::NegDouble:
        // NegDouble dest src : dest = -src (FoxDoubles)
        setReg(instr.NegInt.dest, -getReg<FoxDouble>(instr.NegInt.src));
        continue;
      case Opcode::EqInt:
        // EqInt dest lhs rhs: dest = (lhs == rhs) 
        //          (lhs/rhs: FoxInts, dest: raw)
        TRIVIAL_TAC_BINOP_IMPL(EqInt, FoxInt, ==);
        continue;
      case Opcode::LEInt:
        // LEInt dest lhs rhs: dest = (lhs <= rhs) 
        //          (lhs/rhs: FoxInts, dest: raw)
        TRIVIAL_TAC_BINOP_IMPL(LEInt, FoxInt, <=);
        continue;
      case Opcode::LTInt:
        // LTInt dest lhs rhs: dest = (lhs < rhs) 
        //          (lhs/rhs: FoxInts, dest: raw)
        TRIVIAL_TAC_BINOP_IMPL(LTInt, FoxInt, <);
        continue;
      case Opcode::EqDouble:
        // EqDouble dest lhs rhs: dest = (lhs == rhs) 
        //          (lhs/rhs: FoxDoubles, dest: raw)
        TRIVIAL_TAC_BINOP_IMPL(EqDouble, FoxDouble, ==);
        continue;
      case Opcode::LEDouble:
        // LEDouble dest lhs rhs: dest = (lhs <= rhs) 
        //          (lhs/rhs: FoxDoubles, dest: raw)
        TRIVIAL_TAC_BINOP_IMPL(LEDouble, FoxDouble, <=);
        continue;
      case Opcode::LTDouble:
        // LTDouble dest lhs rhs: dest = (lhs < rhs) 
        //          (lhs/rhs: FoxDoubles, dest: raw)
        TRIVIAL_TAC_BINOP_IMPL(LTDouble, FoxDouble, <);
        continue;
      case Opcode::GEDouble:
        // GEDouble dest lhs rhs: dest = (lhs >= rhs) 
        //          (lhs/rhs: FoxDoubles, dest: raw)
        TRIVIAL_TAC_BINOP_IMPL(GEDouble, FoxDouble, >=);
        continue;
      case Opcode::GTDouble:
        // GTDouble dest lhs rhs: dest = (lhs > rhs) 
        //          (lhs/rhs: FoxDoubles, dest: raw)
        TRIVIAL_TAC_BINOP_IMPL(GTDouble, FoxDouble, >);
        continue;
      case Opcode::LOr:
        // LOr dest lhs rhs: dest = (lhs || rhs) (raw registers)
        setReg(instr.LOr.dest, 
              (getReg(instr.LOr.lhs) || getReg(instr.LOr.rhs)));
        continue;
      case Opcode::LAnd:
        // LAnd dest lhs rhs: dest = (lhs && rhs) (raw registers)
        setReg(instr.LAnd.dest, 
              (getReg(instr.LAnd.lhs) && getReg(instr.LAnd.rhs)));
        continue;
      case Opcode::LNot:
        // LNot dest srrhs: dest = !src
        setReg(instr.LNot.dest, !getReg(instr.LNot.src));
        continue;
      case Opcode::JumpIf:
        // JumpIf condReg offset : Add offset (int16) to pc 
        //    if condReg != 0
        if(getReg(instr.JumpIf.condReg))
          curInstr_ += instr.JumpIf.offset;
        continue;
      case Opcode::JumpIfNot:
        // JumpIfNot condReg offset : Add offset (int16) to pc 
        //    if condReg == 0
        if(!getReg(instr.JumpIfNot.condReg))
          curInstr_ += instr.JumpIfNot.offset;
        continue;
      case Opcode::Jump:
        // Jump offset: Add offset (int16) to pc
        curInstr_ += instr.Jump.offset;
        continue;
      case Opcode::IntToDouble:
        // IntToDouble dest srrhs: dest = (src as FoxDouble) (srrhs: FoxInt)
        setReg(instr.IntToDouble.dest, 
               FoxDouble(getReg<FoxInt>(instr.IntToDouble.src)));
        continue;
      case Opcode::DoubleToInt:
        // DoubleToInt dest srrhs: dest = (src as FoxInt) (srrhs: FoxDouble)
        setReg(instr.DoubleToInt.dest, 
               FoxInt(getReg<FoxDouble>(instr.DoubleToInt.src)));
        continue;
      case Opcode::Copy:
        // Copy dest src : dest = src
        setReg(instr.Copy.dest, getReg(instr.Copy.src));
        continue;
      case Opcode::LoadIntK:
        // Copies the integer constant 'kID' into the register 'dest'
        setReg(instr.LoadIntK.dest, 
               bcModule.getIntConstant(instr.LoadIntK.kID));
        continue;
      case Opcode::LoadDoubleK:
        // Copies the double constant 'kID' into the register 'dest'
        setReg(instr.LoadDoubleK.dest, 
               bcModule.getDoubleConstant(instr.LoadDoubleK.kID));
        continue;
      case Opcode::RetVoid:
        return nullptr;
      case Opcode::Ret:
        return getRegPtr(instr.Ret.reg);
      default:
        fox_unreachable("illegal or unimplemented instruction found");
    }
    #undef TRIVIAL_TAC_BINOP_IMPL
  } while(++curInstr_);
  fox_unreachable("execution did not terminate correctly: "
    "reached the end of the buffer before a return instr");
}

std::size_t VM::getPCIndex() const {
  return std::distance(instrsBeg_, curInstr_);
}

const Instruction* VM::getPC() const {
  return curInstr_;
}

ArrayRef<VM::reg_t> VM::getRegisterStack() const {
  return regStack_;
}

MutableArrayRef<VM::reg_t> VM::getRegisterStack() {
  return regStack_;
}

void VM::setupIterators(ArrayRef<Instruction> instrs) {
  instrsBeg_ = curInstr_ = instrs.begin();
}
