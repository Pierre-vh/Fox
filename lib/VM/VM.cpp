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

VM::Register* VM::call(BCFunction& func, MutableArrayRef<Register> args) {
  /// Copy the args into registers r0 -> rN
  if(args.size()) {
    regaddr_t k = 0;
    for (auto arg : args)
      getReg(k++) = arg;
  }
  /// Run the bytecode
  Register* rtr = run(func.getInstructions());
  /// Copy the args back if needed
  if(args.size()) {
    for (regaddr_t k = 0; k < args.size(); k++)
      args[k] = getReg(k);
  }
  return rtr;
}

VM::Register* VM::run(ArrayRef<Instruction> instrs) {
  pc_ = instrs.begin();
  Instruction instr;
  do {
    // Fetch the current instruction
    instr = *pc_;
    // Macros used to implement repetitive operations
    #define TRIVIAL_TAC_BINOP_IMPL(ID, MEMB, OP)\
      getReg(instr.ID.dest).MEMB = \
      getReg(instr.ID.lhs).MEMB OP getReg(instr.ID.rhs).MEMB
    #define TRIVIAL_TAC_COMP_IMPL(ID, MEMB, OP)\
      getReg(instr.ID.dest).raw = \
      getReg(instr.ID.lhs).MEMB OP getReg(instr.ID.rhs).MEMB
    switch (instr.opcode) {
      case Opcode::NoOp: 
        // NoOp: no-op: do nothing.
        continue;
      case Opcode::StoreSmallInt:
        // StoreSmallInt dest value: Stores value in dest (value: int16)
        getReg(instr.StoreSmallInt.dest).intVal = instr.StoreSmallInt.value;
        continue;
      case Opcode::AddInt: 
        // AddInt dest lhs rhs: dest = lhs + rhs (FoxInts)
        TRIVIAL_TAC_BINOP_IMPL(AddInt, intVal, +);
        continue;
      case Opcode::AddDouble:
        // AddDouble dest lhs rhs: dest = lhs + rhs (FoxDoubles)
        TRIVIAL_TAC_BINOP_IMPL(AddDouble, doubleVal, +);
        continue;
      case Opcode::SubInt:
        // SubInt dest lhs rhs: dest = lhs - rhs (FoxInts)
        TRIVIAL_TAC_BINOP_IMPL(SubInt, intVal, -);
        continue;
      case Opcode::SubDouble:
        // SubDouble dest lhs rhs: dest = lhs - rhs (FoxDoubles)
        TRIVIAL_TAC_BINOP_IMPL(SubDouble, doubleVal, -);
        continue;
      case Opcode::MulInt:
        // DivInt dest lhs rhs: dest = lhs * rhs (FoxInts)
        TRIVIAL_TAC_BINOP_IMPL(MulInt, intVal, *);
        continue;
      case Opcode::MulDouble:
        // SubDouble dest lhs rhs: dest = lhs * rhs (FoxDoubles)
        TRIVIAL_TAC_BINOP_IMPL(MulDouble, doubleVal, *);
        continue;
      case Opcode::DivInt:
        // DivInt dest lhs rhs: dest = lhs / rhs (FoxInts)
        // TO-DO: Handle division by zero with something else than an assert
        assert(getReg(instr.DivInt.rhs).intVal && "division by zero");
        TRIVIAL_TAC_BINOP_IMPL(DivInt, intVal, /);
        continue;
      case Opcode::DivDouble:
        // SubDouble dest lhs rhs: dest = lhs / rhs (FoxDoubles)
        // TO-DO: Handle division by zero with something else than an assert
        assert(getReg(instr.DivDouble.rhs).doubleVal && "division by zero");
        TRIVIAL_TAC_BINOP_IMPL(DivDouble, doubleVal, /);
        continue;
      case Opcode::ModInt:
        // ModInt dest lhs rhs: dest = lhs % rhs (FoxInts)
        // TO-DO: Handle modulo by zero with something else than an assert
        assert(getReg(instr.ModInt.rhs).intVal && "Modulo by zero");
        TRIVIAL_TAC_BINOP_IMPL(ModInt, intVal, %);
        continue;
      case Opcode::ModDouble:
        // ModDouble dest lhs rhs: dest = lhs % rhs (FoxDoubles)
        // TO-DO: Handle modulo by zero with something else than an assert
        assert(getReg(instr.ModDouble.rhs).doubleVal && "Modulo by zero");
        getReg(instr.ModDouble.dest).doubleVal = static_cast<FoxDouble>(
          std::fmod(
            getReg(instr.ModDouble.lhs).doubleVal, 
            getReg(instr.ModDouble.rhs).doubleVal
          )
        );
        continue;
      case Opcode::PowInt:
        // PowInt dest lhs rhs: dest = pow(lhs, rhs) (FoxInts)
        getReg(instr.PowInt.dest).intVal = static_cast<FoxInt>(
          std::pow(
            getReg(instr.PowInt.lhs).intVal, 
            getReg(instr.PowInt.rhs).intVal
          )
        );
        continue;
      case Opcode::PowDouble:
        // PowDouble dest lhs rhs: dest = pow(lhs, rhs) (FoxDoubles)
        getReg(instr.PowDouble.dest).doubleVal = static_cast<FoxDouble>(
          std::pow(
            getReg(instr.PowDouble.lhs).doubleVal, 
            getReg(instr.PowDouble.rhs).doubleVal
          )
        );
        continue;
      case Opcode::NegInt:
        // NegInt dest src : dest = -src (FoxInts)
        getReg(instr.NegInt.dest).intVal = -getReg(instr.NegInt.src).intVal;
        continue;
      case Opcode::NegDouble:
        // NegDouble dest src : dest = -src (FoxDoubles)
        getReg(instr.NegDouble.dest).doubleVal = 
          -getReg(instr.NegDouble.src).doubleVal;
        continue;
      case Opcode::EqInt:
        // EqInt dest lhs rhs: dest = (lhs == rhs) 
        //          (lhs/rhs: FoxInts, dest: raw)
        TRIVIAL_TAC_COMP_IMPL(EqInt, intVal, ==);
        continue;
      case Opcode::LEInt:
        // LEInt dest lhs rhs: dest = (lhs <= rhs) 
        //          (lhs/rhs: FoxInts, dest: raw)
        TRIVIAL_TAC_COMP_IMPL(LEInt, intVal, <=);
        continue;
      case Opcode::LTInt:
        // LTInt dest lhs rhs: dest = (lhs < rhs) 
        //          (lhs/rhs: FoxInts, dest: raw)
        TRIVIAL_TAC_COMP_IMPL(LTInt, intVal, <);
        continue;
      case Opcode::EqDouble:
        // EqDouble dest lhs rhs: dest = (lhs == rhs) 
        //          (lhs/rhs: FoxDoubles, dest: raw)
        TRIVIAL_TAC_COMP_IMPL(EqDouble, doubleVal, ==);
        continue;
      case Opcode::LEDouble:
        // LEDouble dest lhs rhs: dest = (lhs <= rhs) 
        //          (lhs/rhs: FoxDoubles, dest: raw)
        TRIVIAL_TAC_COMP_IMPL(LEDouble, doubleVal, <=);
        continue;
      case Opcode::LTDouble:
        // LTDouble dest lhs rhs: dest = (lhs < rhs) 
        //          (lhs/rhs: FoxDoubles, dest: raw)
        TRIVIAL_TAC_COMP_IMPL(LTDouble, doubleVal, <);
        continue;
      case Opcode::GEDouble:
        // GEDouble dest lhs rhs: dest = (lhs >= rhs) 
        //          (lhs/rhs: FoxDoubles, dest: raw)
        TRIVIAL_TAC_COMP_IMPL(GEDouble, doubleVal, >=);
        continue;
      case Opcode::GTDouble:
        // GTDouble dest lhs rhs: dest = (lhs > rhs) 
        //          (lhs/rhs: FoxDoubles, dest: raw)
        TRIVIAL_TAC_COMP_IMPL(GTDouble, doubleVal, >);
        continue;
      case Opcode::LOr:
        // LOr dest lhs rhs: dest = (lhs || rhs) (raw registers)
        getReg(instr.LOr.dest).raw =
          (getReg(instr.LOr.lhs).raw || getReg(instr.LOr.rhs).raw);
        continue;
      case Opcode::LAnd:
        // LAnd dest lhs rhs: dest = (lhs && rhs) (raw registers)
        getReg(instr.LAnd.dest).raw =
          (getReg(instr.LAnd.lhs).raw && getReg(instr.LAnd.rhs).raw);
        continue;
      case Opcode::LNot:
        // LNot dest src: dest = !src
        getReg(instr.LNot.dest).raw = !getReg(instr.LNot.src).raw;
        continue;
      case Opcode::JumpIf:
        // JumpIf condReg offset : Add offset (int16) to pc 
        //    if condReg != 0
        if(getReg(instr.JumpIf.condReg).raw)
          pc_ += instr.JumpIf.offset;
        continue;
      case Opcode::JumpIfNot:
        // JumpIfNot condReg offset : Add offset (int16) to pc 
        //    if condReg == 0
        if(!getReg(instr.JumpIfNot.condReg).raw)
          pc_ += instr.JumpIfNot.offset;
        continue;
      case Opcode::Jump:
        // Jump offset: Add offset (int16) to pc
        pc_ += instr.Jump.offset;
        continue;
      case Opcode::IntToDouble:
        // IntToDouble dest src: dest = (src as FoxDouble) (src FoxInt)
        getReg(instr.IntToDouble.dest).doubleVal =
          FoxDouble(getReg(instr.IntToDouble.src).intVal);
        continue;
      case Opcode::DoubleToInt:
        // DoubleToInt dest src: dest = (src as FoxInt) (src: FoxDouble)
        getReg(instr.DoubleToInt.dest).intVal =
          FoxInt(getReg(instr.DoubleToInt.src).doubleVal);
        continue;
      case Opcode::Copy:
        // Copy dest src : dest = src
        getReg(instr.Copy.dest).raw = getReg(instr.Copy.src).raw;
        continue;
      case Opcode::LoadIntK:
        // Copies the integer constant 'kID' into the register 'dest'
        getReg(instr.LoadIntK.dest).intVal =
          bcModule.getIntConstant(instr.LoadIntK.kID);
        continue;
      case Opcode::LoadDoubleK:
        // Copies the double constant 'kID' into the register 'dest'
        getReg(instr.LoadDoubleK.dest).doubleVal =
          bcModule.getDoubleConstant(instr.LoadDoubleK.kID);
        continue;
      case Opcode::RetVoid:
        return nullptr;
      case Opcode::Ret:
        return getRegPtr(instr.Ret.reg);
      case Opcode::LoadFunc:
        // LoadFunc dest func : loads a reference to the function with the
        //  ID 'func' in 'dest'.
        getReg(instr.LoadFunc.dest).func =
          &(bcModule.getFunction(instr.LoadFunc.func));
        continue;
      case Opcode::CallVoid:
        // CallVoid base : calls a function located in 'base'.
        //  Args are in subsequent registers.
        callFunc(instr.CallVoid.base);
        continue;
      case Opcode::Call:
      {
        // CallVoid base dest : calls a function located in 'base'
        //  Args are in subsequent registers.
        //  Stores the result in 'dest'
        Register* result = callFunc(instr.CallVoid.base);
        assert(result && "function returning void called with "
          "'Call' and not 'CallVoid'");
        getReg(instr.Call.dest) = *result;
        continue;
      }
      default:
        fox_unreachable("illegal or unimplemented instruction found");
    }
    #undef TRIVIAL_TAC_BINOP_IMPL
    #undef TRIVIAL_TAC_COMP_IMPL
  } while(++pc_);
  fox_unreachable("execution did not terminate correctly: "
    "reached the end of the buffer before a return instr");
}

const Instruction* VM::getPC() const {
  return pc_;
}

ArrayRef<VM::Register> VM::getRegisterStack() const {
  return regStack_;
}

MutableArrayRef<VM::Register> VM::getRegisterStack() {
  return regStack_;
}

VM::Register* VM::callFunc(regaddr_t base) {
  // Fetch a pointer to the base
  Register* basePtr = getRegPtr(base);
  // Fetch the BCFunction
  BCFunction* fn = *reinterpret_cast<BCFunction**>(basePtr);
  assert(fn && "func is null");

  ///////////////////////////////////////////////////////////////////
  // TODO: Realloc the stack if needed ( if the func needs more regs
  // than what's available on the regStack)
  ///////////////////////////////////////////////////////////////////

  // Backup the instrBeg and curInstr ptrs

  // Backup the current register window position
  Register* previousBase = baseReg_;
  // Slide the window so it begins at basePtr+1
  baseReg_ = basePtr+1;

  // Backup the current program counter and the instrsBeg_
  // pointer.
  auto oldPC = pc_;

  // Run
  Register* rtrReg = run(fn->getInstructions());

  // Restore the window and pc pointers
  baseReg_ = previousBase;
  pc_ = oldPC;

  return rtrReg;
}

namespace {
  // Template machinery to call builtins
  template<typename Ty>
  struct ConvertReg {
    static void set(Ty&, VM::Register) {
      static_assert(false, "This type is not supported by the Fox FFI!");
    }

    static VM::Register get(const Ty&) {
      static_assert(false, "This type is not supported by the Fox FFI!");
    }
  };
  #define REG_CONVERT(TYPE, SET_IMPL, GET_IMPL)\
  template<> struct ConvertReg<TYPE>\
    { static void set(TYPE& dest, VM::Register reg) { SET_IMPL; } \
      static VM::Register get(const TYPE& value) { GET_IMPL; } }
  REG_CONVERT(FoxInt,      dest = reg.intVal, value);
  REG_CONVERT(FoxDouble,   dest = reg.doubleVal, value);
  REG_CONVERT(bool,        dest = reg.raw, value);
  REG_CONVERT(FoxChar,     dest = reg.raw, value);
  #undef REG_CONVERT

  namespace detail
  {
      template<int... Is>
      struct seq { };

      template<int N, int... Is>
      struct gen_seq : gen_seq<N - 1, N - 1, Is...> { };

      template<int... Is>
      struct gen_seq<0, Is...> : seq<Is...> { };

      template<typename T, typename F, int... Is>
      void for_each(T&& t, F f, seq<Is...>) {
        auto l = { (f(std::get<Is>(t)), 0)... };
      }
  }

  template<typename... Ts, typename F>
  void for_each_in_tuple(std::tuple<Ts...>& t, F f) {
    detail::for_each(t, f, detail::gen_seq<sizeof...(Ts)>());
  }

  struct Functor {
    VM::Register* reg;

    template<typename T>
    void operator()(T& val) {
      ConvertReg<T>::set(val, *(reg++));
    }
  };

  template<typename Rtr, typename ... Args> 
  VM::Register doCall(Rtr(*fn)(Args...), const std::tuple<Args...>& args) {
    return ConvertReg<Rtr>::get(fn(std::get<Args>(args)...));
  }

  template<typename ... Args> 
  VM::Register doCall(void(*fn)(Args...), const std::tuple<Args...>& args) {
    fn(std::get<Args>(args)...);
    return VM::Register();
  }

  VM::Register callBuiltinImpl(VM::Register*, void(*fn)()) {
    fn();
    return VM::Register();
  }

  template<typename Rtr, typename ... Args>
  VM::Register callBuiltinImpl(VM::Register* args, Rtr(*fn)(Args...)) {
    using ArgTupleType = std::tuple<Args...>;
    ArgTupleType argTuple;
    Functor functor;
    functor.reg = args;
    for_each_in_tuple(argTuple, functor);
    return doCall(fn, argTuple);
  }

  template<typename Rtr>
  VM::Register callBuiltinImpl(VM::Register*, Rtr(*fn)()) {
    fn();
    return doCall(fn);
  }
}

VM::Register VM::callBuiltinFunc(BuiltinID id) {
  switch (id) {
    #define BUILTIN(FUNC, FOX) case BuiltinID::FUNC:\
    callBuiltinImpl(getRegPtr(0), builtin::FUNC); break;
    #include "Fox/Common/Builtins.def"
    default:
      fox_unreachable("Unknown BuiltinID");
  }
  /// TODO
  return VM::Register();
}
