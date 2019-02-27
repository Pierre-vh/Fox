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
#include <functional>

namespace fox {
  // AlignedBoxedValue is a wrapper around a single value that is "boxed"
  // inside this object. The second template argument is an alignement value,
  // so you can choose the alignement that this value should have.
  //
  // Note that this struct *only* contains the value, without extra fields,
  // so it should be safe to static_cast to the primitive type.
  template<typename ValueType, unsigned Alignement>
  struct alignas(Alignement) AlignedBoxedValue {
    AlignedBoxedValue() = default;
    AlignedBoxedValue(ValueType value) : value(value) {}
    ValueType value;
  };

  // The "Value" class.
  //
  // It is preffered to pass this class by reference to minimize copies.
  class Value {
    public:
      Value() = default;
      explicit Value(FoxInt value);
      explicit Value(FoxDouble value);
      explicit Value(FoxChar value);

      // Copy constructor
      Value(const Value& value);

      // Move constructor
      Value(Value&& value);

      // Assignement
      Value& operator=(const Value& value);

      ~Value();

      // Sets the int value of this Value, resetting
      // the value first if it isn't an int.
      void setInt(FoxInt value);
      void setDouble(FoxDouble value);
      void setChar(FoxChar value);

      // Returns the value. 
      //
      // isInt() must return true.
      FoxInt getInt() const;

      // Returns the value. 
      //
      // isDouble() must return true.
      FoxDouble getDouble() const;

      // Returns the value. 
      //
      // isChar() must return true.
      FoxChar getChar() const;

      bool isNull() const;
      bool isInt() const;
      bool isDouble() const;
      bool isChar() const;

      void reset();

    private:
      // Returns true if this Value is the unique owner of its data.
      bool isUniquelyOwned() const;

      // Deletes the owned value.
      //
      // The value must not be null and must be uniquely
      // owned.
      void deleteOwnedValue();

      // Makes this value a deep copy of "other"
      void deepCopy(const Value& other);

      template<typename Ty>
      using BoxedPrimtive = AlignedBoxedValue<Ty, 8>;

      using IntBox = BoxedPrimtive<FoxInt>;
      using DoubleBox = BoxedPrimtive<FoxDouble>;
      using CharBox = BoxedPrimtive<FoxChar>;

      llvm::PointerUnion3<
        BoxedPrimtive<FoxInt>*,
        BoxedPrimtive<FoxDouble>*,
        BoxedPrimtive<FoxChar>*
      > pointers_;
  };
}