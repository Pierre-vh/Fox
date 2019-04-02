//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : TypeWalker.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file declares the TypeWalker class
//----------------------------------------------------------------------------//

#include "Fox/AST/ASTFwdDecl.hpp"

namespace fox {
  class Type;

  /// The TypeWalker, used to traverse a Type hierarchy
  class TypeWalker {
    public:
      /// Walks a Type.
      /// \param type the type to walk
      /// \returns true if the traversal was successful, false if it
      /// was terminated early.
      bool walk(Type type);

      /// Called when first visiting a type before visiting its children. 
      /// \returns true to visit this type's children, false otherwise.
      virtual bool handleTypePre(Type type);

      /// Called after visiting a type's children
      /// \returns true to continue the walk, false to terminate it.
      virtual bool handleTypePost(Type type);
  };
}