//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : BuiltinsTypes.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the interface used to retrieve the type of a builtin
// function.
//----------------------------------------------------------------------------//

#pragma once

#include "Fox/Common/Builtins.hpp"

namespace fox {
  class ASTContext;  
  class Type;

  /// \returns the type of the builtin identified by \p id. Types will be
  ///          created in \p ctxt
  Type getTypeOfBuiltin(ASTContext& ctxt, BuiltinID id);
}