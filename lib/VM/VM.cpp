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
#include "Fox/Common/Builtins.hpp"
#include "Fox/Common/Objects.hpp"
#include "Fox/Common/STLExtras.hpp"
#include <cmath>
#include <type_traits>
#include <iterator>
#include <tuple>

using namespace fox;

VM::VM(BCModule& theModule) : bcModule(theModule) {
  /// The base register will simply be the first register in the
  /// stack.
  baseReg_ = regStack_.data();
}

/// Out of line because unique_ptr needs to see the definition of
/// StringObject and others.
VM::~VM() = default;

VM::Register VM::call(BCFunction& func, MutableArrayRef<Register> args) {
  /// Copy the args into registers r0 -> rN
  if(args.size()) {
    regaddr_t k = 0;
    for (auto arg : args)
      getReg(k++) = arg;
  }
  /// Run the bytecode
  Register rtr = run(func.getInstructions());
  /// Copy the args back if needed
  if(args.size()) {
    for (regaddr_t k = 0; k < args.size(); k++)
      args[k] = getReg(k);
  }
  return rtr;
}

VM::Register VM::run(ArrayRef<Instruction> instrs) {
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
      case Opcode::NewString:
        // Create a new string and put a pointer to it in 'dest'
        getReg(instr.NewString.dest).object = newStringObject();
        continue;
      case Opcode::LoadStringK:
        // Create a new string from a string constant 'kID' stored in bcModule
        // and put a pointer to it in 'dest'
        getReg(instr.LoadStringK.dest).object =
          newStringObjectFromK(instr.LoadStringK.kID);
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
      case Opcode::RetVoid:
        return VM::Register();
      case Opcode::Ret:
        return getReg(instr.Ret.reg);
      case Opcode::LoadFunc:
        // LoadFunc dest func : loads a reference to the function with the
        //  ID 'func' in 'dest'.
        getReg(instr.LoadFunc.dest).funcRef =
          &(bcModule.getFunction(instr.LoadFunc.func));
        continue;
      case Opcode::LoadBuiltinFunc:
        // LoadBuiltinFunc dest id : loads a reference to the
        //                           builtin 'id' in 'dest'
        getReg(instr.LoadBuiltinFunc.dest).funcRef = instr.LoadBuiltinFunc.id;
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
        getReg(instr.Call.dest) = callFunc(instr.CallVoid.base);
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

LLVM_ATTRIBUTE_RETURNS_NONNULL LLVM_ATTRIBUTE_RETURNS_NOALIAS
StringObject* VM::newStringObject(string_view str) {
  stringObjects_.emplace_back(std::make_unique<StringObject>(str));
  StringObject* ptr = stringObjects_.back().get();
  assert(ptr && "Pointer to allocated object is nullptr");
  return ptr;
}

LLVM_ATTRIBUTE_RETURNS_NONNULL LLVM_ATTRIBUTE_RETURNS_NOALIAS
StringObject* VM::newStringObjectFromK(constant_id_t kID) {
  const auto& str = bcModule.getStringConstant(kID);
  stringObjects_.emplace_back(std::make_unique<StringObject>(str));
  StringObject* ptr = stringObjects_.back().get();
  assert(ptr && "Pointer to allocated object is nullptr");
  return ptr;
}

MutableArrayRef<VM::Register> VM::getRegisterStack() {
  return regStack_;
}

VM::Register VM::callFunc(regaddr_t base) {
  Register* basePtr = getRegPtr(base);
  FunctionRef& fnRef = basePtr->funcRef;

  // Backup the instrBeg and curInstr ptrs

  // Backup the current register window position
  Register* previousBase = baseReg_;
  // Slide the window so it begins at basePtr+1
  baseReg_ = basePtr+1;

  // Backup the current program counter and the instrsBeg_
  // pointer.
  auto oldPC = pc_;

  Register rtr;
  if(fnRef.isBCFunction()) 
    rtr = run(fnRef.getBCFunction()->getInstructions());
  else if(fnRef.isBuiltinID()) 
    rtr = callBuiltinFunc(fnRef.getBuiltinID());
  else 
    fox_unreachable("unknown FunctionRef kind");

  // Restore the window and pc pointers
  baseReg_ = previousBase;
  pc_ = oldPC;

  return rtr;
}

namespace {
  //--------------------------------------------------------------------------//
  // Argument conversion and call logic
  //--------------------------------------------------------------------------//

  /// Utility class used to perform conversions betwween a VM::Register
  /// and another type.
  template<typename Ty>
  struct RegCast {
    static Ty regToType(VM::Register) {
      static_assert(false, "This type is not supported by the Fox FFI!");
    }

    static VM::Register typeToReg(Ty) {
      static_assert(false, "This type is not supported by the Fox FFI!");
    }
  };

  #define REG_CONVERT(TYPE, FROM_EXPR, TO_EXPR)                                   \
  template<> struct RegCast<TYPE>                                                 \
    {                                                                             \
      static TYPE regToType(VM::Register reg)                                     \
              { return FROM_EXPR; }                                               \
      static VM::Register typeToReg(TYPE value)                                   \
             { return VM::Register(TO_EXPR); }                                    \
    }

  REG_CONVERT(FoxInt        , reg.intVal     
                            , value);
  REG_CONVERT(FoxDouble     , reg.doubleVal  
                            , value);
  REG_CONVERT(bool          , reg.raw        
                            , std::uint64_t(value));
  REG_CONVERT(FoxChar       , reg.raw        
                            , std::uint64_t(value));
  REG_CONVERT(StringObject* , cast<StringObject>(reg.object)     
                            , value);
  #undef REG_CONVERT

  template<typename Ty>
  struct ArgCaster {
    static Ty cast(VM&, VM::Register*& base) {
      return RegCast<Ty>::regToType(*(base++));
    }
  };

  template<>
  struct ArgCaster<VM&> {
    /// Just returns the VM instance
    static VM& cast(VM& vm, VM::Register*&) {
      return vm;
    }
  };

  //--------------------------------------------------------------------------//
  // doBuiltinCall overloads
  //--------------------------------------------------------------------------//

  /// Calls a builtin that takes arguments and returns something
  template<typename Rtr, typename ... Args>
  VM::Register doBuiltinCall(VM& vm, VM::Register* base, Rtr(*fn)(Args...)) {
    // Create a tuple of arguments
    std::tuple<Args...> args{ArgCaster<Args>::cast(vm, base)...};
    // Apply
    auto result = apply(fn, args);
    // Cast the return value to a VM::Reg
    return RegCast<Rtr>::template typeToReg(result);
  }

  /// Calls a builtin that takes arguments but doesn't return anything
  template<typename ... Args>
  VM::Register doBuiltinCall(VM& vm, VM::Register* base, void(*fn)(Args...)) {
    // Create a tuple of arguments
    std::tuple<Args...> args{ArgCaster<Args>::cast(vm, base)...};
    // Apply
    apply(fn, args);
    // Simply returns null register for void function calls
    return VM::Register();
  }

  /// Calls a simple builtin that doesn't take any argument and
  /// just returns a value
  template<typename Rtr>
  VM::Register doBuiltinCall(VM&, VM::Register*, Rtr(*fn)()) {
    return RegCast<Rtr>::template from_reg(fn());
  }

  /// Calls a simple builtin that doesn't take any argument and does not
  /// return anything.
  VM::Register doBuiltinCall(VM&, VM::Register*, void(*fn)()) {
    fn();
    return VM::Register();
  }
}

VM::Register VM::callBuiltinFunc(BuiltinID id) {
  switch (id) {
    #define PUBLIC_BUILTIN(FUNC, FOX)\
      case BuiltinID::FUNC:   \
        return doBuiltinCall(*this, getRegPtr(0), builtin::FUNC);
    #include "Fox/Common/Builtins.def"
    default:
      fox_unreachable("Unknown BuiltinID");
  }
}
