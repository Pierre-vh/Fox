//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : FoxAny.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#pragma once

#include "FoxTypes.hpp"

namespace fox {
  class Object;

  /// Untagged union that can contain a value of any FoxType.
  /// This is always 64 bits in size, even in 32 bits builds.
  union FoxAny {
    FoxAny()                           : raw(0) {}
    explicit FoxAny(std::uint64_t raw) : raw(raw) {}
    explicit FoxAny(FoxInt v)          : intVal(v) {}
    explicit FoxAny(FoxDouble v)       : doubleVal(v) {}
    explicit FoxAny(bool v)            : boolVal(v) {}
    explicit FoxAny(FoxChar v)         : charVal(v) {}
    explicit FoxAny(Object* v)         : objectVal(v) {}

    template<typename T>
    T get() const = delete;

    /// Templated getter that returns the intValue of this union.
    /// This is not a checked operation, so if the union doesn't
    /// contain a FoxInt, this is call will result in UB.
    template<>
    FoxInt get<FoxInt>() const        { return intVal;  }

    /// Templated getter that returns the doubleValue of this union.
    /// This is not a checked operation, so if the union doesn't
    /// contain a FoxDouble, this is call will result in UB.
    template<>
    FoxDouble get<FoxDouble>() const  { return doubleVal; }

    /// Templated getter that returns the boolVal of this union.
    /// This is not a checked operation, so if the union doesn't
    /// contain a FoxBool, this is call will result in UB.
    template<>
    bool get<bool>() const            { return boolVal; }

    /// Templated getter that returns the charVal of this union.
    /// This is not a checked operation, so if the union doesn't
    /// contain a FoxChar, this is call will result in UB.
    template<>
    FoxChar get<FoxChar>() const      { return charVal; }

    /// Templated getter that returns the objectVal of this union.
    /// This is not a checked operation, so if the union doesn't
    /// contain a Object*, this is call will result in UB.
    template<>
    Object* get<Object*>() const      { return objectVal; }

    /// the raw value of the element
    std::uint64_t raw;
    /// Integer elements
    FoxInt    intVal;
    FoxInt    intVal;
    /// Floating-point elements
    FoxDouble doubleVal;
    /// Boolean elements
    bool      boolVal;
    /// Char elements
    FoxChar   charVal;
    /// Object elements
    Object*   objectVal;
  };

  static_assert(sizeof(FoxAny) == 8, "Size of FoxAny is not 64 bits");
}
