//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : Objects.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the Object hierarchy, which is used to represent
// Fox Objects such as Strings and Arrays.
//----------------------------------------------------------------------------//

#pragma once

#include "FoxTypes.hpp"
#include "FoxAny.hpp"
#include "string_view.hpp"
#include "LLVM.hpp"
#include <cstddef>
#include <string>
#include <vector>

namespace fox {
  enum class ObjectKind : std::uint8_t {
    #define OBJECT(CLASS) CLASS,
    #include "Objects.def"
  };

  /// The base Object class
  class Object {
    public:
      ObjectKind getKind() const;

      // TODO: Allocation methods (custom operator new) ?
    protected:
      Object(ObjectKind kind);

    private:
      ObjectKind kind_;
  };

  /// StringObject is an immutable UTF8 String.
  ///   TODO: Trail-allocate the string's data once I get a proper allocator in
  ///         the VM.
  class StringObject : public Object {
    public:
      /// creates an empty StringObject
      StringObject();

      /// creates a StringObject from a pre-existing string_view
      StringObject(string_view value);

      /// \returns a const reference to the underlying string
      const std::string& str() const;

      /// \returns the size of the string in UTF8 codepoints.
      std::size_t length() const;
      /// \returns the size of the string in bytes
      std::size_t numBytes() const;

      FoxChar getChar(std::size_t n) const;

      static bool classof(const Object* obj) {
        return obj->getKind() == ObjectKind::StringObject;
      }

    private:
      const std::string str_;
  };

  /// ArrayObject is a dynamic, untyped array.
  /// It is intended to store homogenous data -> it doesn't provide support
  /// for storing both value/reference types at the same time. 
  /// (It has a single "containsReference" tag, and not a full "reference map")
  class ArrayObject : public Object {
    public:
      using ElemT = FoxAny;
      using ArrayT = std::vector<ElemT>;

      /// Creates an empty array
      /// \param containsReferences true if this array will contain 
      ///                           reference types, false otherwise.
      /// \param minCapacity the minimum capacity that the array should have
      ///                     (enough space will be reserved to store 
      ///                      \p minCapacity elems)
      ArrayObject(bool containsReferences, std::size_t minCapacity = 0);

      void append(ElemT elem);

      ElemT& operator[](std::size_t idx);
      const ElemT& operator[](std::size_t idx) const;

      /// Removes the last element from the array
      void pop();

      /// \returns the first element of the array
      ElemT front();
      /// \returns the last element of the array
      ElemT back();

      /// \returns the size of the array
      std::size_t size() const;

      /// removes every element inside the array
      void reset();

      /// void pop_back();
      /// void erase(std::size_t start, std::size_t num);
      /// Element get(std::size_t elem);

      /// \returns true if this array contains references types.
      bool containsReferences() const;

      /// \returns a reference to the internal array of the ArrayObject
      ArrayT& data();
      /// \returns a const reference to the internal array of the ArrayObject
      const ArrayT& data() const;

      static bool classof(const Object* obj) {
        return obj->getKind() == ObjectKind::ArrayObject;
      }

    private:
      const bool containsReferences_ : 1;
      ArrayT data_;
  };
}