//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : BCBuilder.hpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  This file declares the BCBuilder interface. It is a helper class used to
//  build bytecode buffers.
//----------------------------------------------------------------------------//

#pragma once

#include "BCUtils.hpp"
#include "Fox/BC/BCModule.hpp"
#include "Fox/Common/LLVM.hpp"
#include "Fox/Common/StableVectorIterator.hpp"
#include <cstdint>

namespace fox {
  class BCBuilder {
    public:
      /// A 'stable' iterator for the instruction buffer
      using StableInstrIter = StableVectorIterator<InstructionVector>;

      BCBuilder(InstructionVector& vector);

      #define TERNARY_INSTR(ID, I1, T1, I2, T2, I3, T3)\
        StableInstrIter create##ID##Instr(T1 I1, T2 I2, T3 I3);
      #define BINARY_INSTR(ID, I1, T1, I2, T2)\
        StableInstrIter create##ID##Instr(T1 I1, T2 I2);
      #define UNARY_INSTR(ID, I1, T1)\
        StableInstrIter create##ID##Instr(T1 I1);
      #define SIMPLE_INSTR(ID)\
        StableInstrIter create##ID##Instr();
      #include "Instruction.def"

      /// erases all instructions in the range [beg, end)
      void truncate_instrs(StableInstrIter beg);

      /// \returns true if 'it' == getLastInstrIter()
      bool isLastInstr(StableInstrIter it) const;

      /// \returns an iterator to the last instruction inserted
      /// in the buffer.
      StableInstrIter getLastInstrIter();

      /// Removes the last instruction added to this module.
      void popInstr();

      /// The Instruction Buffer that we are building.
      InstructionVector& vector;

    private:
      StableInstrIter insert(Instruction instr);
  };
}