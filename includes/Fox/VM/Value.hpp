//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : Value.hpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  This file declares the Value class. This class is used to represent
//  values inside the Virtual Machine.
//----------------------------------------------------------------------------//

#include "llvm/ADT/PointerUnion.h"
#include "Fox/Common/LLVM.hpp"
#include "Fox/Common/Typedefs.hpp"

namespace fox {
  // AlignedBoxedValue is a wrapper around a single value that is "boxed"
  // inside this object. The second template argument is an alignement value,
  // so you can choose the alignement that this value should have.
  //
  // Note that this struct *only* contains the value, without extra fields,
  // so it should be safe to static_cast to the value type.
  template<typename ValueType, unsigned Alignement>
  struct alignas(Alignement) AlignedBoxedValue {
    ValueType value;
  };

  class Value {
    public:
      Value() = default;

      bool isNull() const;
      bool isInt() const;
      bool isDouble() const;
      bool isChar() const;

    private:
      template<typename Ty>
      using BoxedPrimtive = AlignedBoxedValue<Ty, 8>;

      // TODO: How will memory be managed?
      llvm::PointerUnion3<
        BoxedPrimtive<FoxInt>*,
        BoxedPrimtive<FoxDouble>*,
        BoxedPrimtive<FoxChar>*
      > pointers_;
  };
}