//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : VMModule.hpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  This file contains the VMModule class, which represents a VM program
//  that can be executed by the Fox VM. 
//----------------------------------------------------------------------------//

#pragma once

#include "Fox/VM/VMUtils.hpp"
#include "Fox/Common/LLVM.hpp"
#include "llvm/ADT/ArrayRef.h"
#include <memory>

namespace fox {
  class VMModule {
    public:
      VMModule();
      ~VMModule();

      void setInstrs(std::unique_ptr<InstructionBuffer> buffer);
      InstructionBuffer* getInstrs();
      const InstructionBuffer* getInstrs() const;
      std::unique_ptr<InstructionBuffer> takeInstrs();

    private:
      std::unique_ptr<InstructionBuffer> instrBuffer_;
  };
}