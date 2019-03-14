//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : TypeWalker.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the TypeWalker class
//----------------------------------------------------------------------------//

#include "Fox/AST/ASTFwdDecl.hpp"

namespace fox {
  class Type;

  // The TypeWalker, used to traverse a Type hierarchy
  class TypeWalker {
    public:
      // Walks a Type.
      // Return true if the traversal was successful, false otherwise.
      bool walk(Type type);

      // Called when first visiting a type before visiting its
      // children. 
      // If the return value is false, the traversal is terminated.
      virtual bool handleTypePre(Type type);

      // Called after visiting a type's children.
      // If the return value is false, the traversal is terminated.
      virtual bool handleTypePost(Type type);
  };
}