//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : SemaLookup.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  This file implements lookup logic for Sema
//----------------------------------------------------------------------------//

#include "Fox/Sema/Sema.hpp"
#include "Fox/AST/DeclContext.hpp"
#include "Fox/AST/Decl.hpp"

using namespace fox;

//----------------------------------------------------------------------------//
// Unqualified Lookup
//----------------------------------------------------------------------------//

namespace {
  using ResultFoundFn = bool (*)(NamedDecl*);

  // Does lookup in a DeclContext.
  //  The lookup stop if:
  //    > onFound(...) returns false
  //  *or*
  //    > if climbParents = false and we are done looking up this DeclContext
  //  *or*
  //    > if we are done searching the whole DeclContext tree.
  void lookupInDeclContext(ResultFoundFn /*onFound*/, DeclContext* /*dc*/, 
    bool /*climbParents*/) {
    // TODO
  }

  // Does lookup in a LocalScope.
  //   > The lookup stop if onFound(...) returns false
  // *or*
  //   > if we are done searching the whole scope tree.
  void lookupInLocalScope(ResultFoundFn /*onFound*/, LocalScope* /*sc*/) {
    // TODO
  }

}

// TODO