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

#include <cstddef>
#include <string>
#include "FoxTypes.hpp"
#include "string_view.hpp"

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

  /// Object representing an immutable UTF8 String.
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

      /// void setChar(std::size_t idx, FoxChar ch)
      /// FoxChar getChar(std::size_t idx)

      /// void setByte(std::size_t idx, char ch);
      /// char getByte(std::size_t idx)

      /// void eraseChars(std::size_t idx, std::size_t end = idx)
      /// void eraseBytes(std::size_t idx, std::size_t end = idx)

      static bool classof(const Object* obj) {
        return obj->getKind() == ObjectKind::StringObject;
      }

    private:
      const std::string str_;
  };
}