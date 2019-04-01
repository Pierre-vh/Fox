//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : Token.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the Token class and TokenKind enum.
//----------------------------------------------------------------------------//

#pragma once

#include "Fox/AST/Identifier.hpp"
#include "Fox/Common/FoxTypes.hpp"
#include "Fox/Common/SourceLoc.hpp"
#include "Fox/Common/LLVM.hpp"
#include "llvm/ADT/SmallVector.h"
#include <cstddef>
#include <iosfwd>

namespace fox {
  class ASTContext;
  class DiagnosticEngine;

  /// TokenKind
  ///    The different kind of tokens that exist
  enum class TokenKind : std::uint8_t {
    #define TOKEN(ID) ID,
    #include "TokenKinds.def"
  };

  /// Token
  ///    Provides information about a lexed token: its string, location and kind.
  struct Token  {
    public:
      using Kind = TokenKind;
      // Creates an invalid token
      Token() = default;
      // Creates a normal token
      Token(Kind kind, string_view str, SourceRange range);

      bool isValid() const;
      explicit operator bool() const;

      bool is(Kind kind) const;

      void dump(std::ostream& out) const;

      const SourceRange range;
      const string_view str;
      const Kind kind = Kind::Invalid;
  };

  /// A Vector of Tokens.
  using TokenVector = SmallVector<Token, 4>;
}
