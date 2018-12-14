//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : SemaLookup.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  This file implements Sema methods related to scopes and name lookup
//  as well as most of the Lookup and Name binding logic.
//----------------------------------------------------------------------------//

#include "Fox/Sema/Sema.hpp"
#include "Fox/AST/DeclContext.hpp"
#include "Fox/AST/Decl.hpp"
#include <functional>

using namespace fox;

//----------------------------------------------------------------------------//
// Unqualified Lookup
//----------------------------------------------------------------------------//

namespace {
  using ResultFoundFn = std::function<bool(NamedDecl*)>;
  using DeclsMap = DeclContext::MapTy;

  // Searches the map. Returns false if onFound requested the search to stop,
  // true if the search terminated normally. 
  bool searchMap(const DeclsMap& map, Identifier id, ResultFoundFn onFound) {
    DeclsMap::const_iterator beg, end;
    std::tie(beg, end) = map.equal_range(id);
    for(auto it = beg; it != end; ++it) {
      if(!onFound(it->second)) return false;
    }
    return true;
  }

  // Does lookup in a DeclContext.
  //  The lookup stop if:
  //    > onFound(...) returns false
  //  *or*
  //    > if we are done searching the whole DeclContext tree.
  void lookupInDeclContext(Identifier id, ResultFoundFn onFound, 
    DeclContext* dc) {
    DeclContext* cur = dc;
    while(cur) {
      if(!searchMap(cur->getDeclsMap(), id, onFound))
        return;
      cur = dc->getParentDeclCtxt();
    }
  }

  // Does lookup in a LocalScope.
  //   > The lookup stop if onFound(...) returns false
  // *or*
  //   > if we are done searching the whole scope tree.
  void lookupInLocalScope(Identifier id, ResultFoundFn onFound, 
    LocalScope* scope) {
    LocalScope* cur = scope;
    while(cur) {
      if(!searchMap(cur->getDeclsMap(), id, onFound))
        return;
      // Climb parent scopes
      cur = scope->getParent();
    }
  }

}

//----------------------------------------------------------------------------//
// Sema methods impl
//----------------------------------------------------------------------------//

void Sema::addToScopeIfLocal(NamedDecl* decl) {
  if(hasLocalScope() && decl->isLocal())
    getLocalScope()->add(decl);
}

void Sema::doUnqualifiedLookup(LookupResult& results, Identifier id,
  bool stopOnFirstResult, bool doOnlyLocalScopeLookup) {

  // Lambda that returns true if we should return early.
  auto shouldReturn = [&]() {
    // If we have one result, return
    if(stopOnFirstResult) {
      assert((results.size() <= 1) && 
        "Should have 0 or 1 result if stopOnFirstResult is true");
      return (results.size() != 0);
    }
    return false;
  };

  // Lambda that handles a result
  auto handleResult = [&](NamedDecl* decl) {
    results.addResult(decl);
    // !shouldReturn() because we must return false
    // if we want to stop looking.
    return !shouldReturn();
  };

  // Check in local scope, if there's one.
  if(hasLocalScope()) {
    LocalScope* scope = getLocalScope();
    // Do the lookup in the local scope
    lookupInLocalScope(id, handleResult, scope);
    // Return if we must
    if(shouldReturn()) return;
  }

  // Check in decl context if allowed to.
  if(!doOnlyLocalScopeLookup) {
    // Check in decl context if allowed to.
    DeclContext* dc = getDeclCtxt();
    // We should ALWAYS have a DC, else, something's broken.
    assert(dc && "No DeclContext available?");
    lookupInDeclContext(id, handleResult, dc);
  }
}

//----------------------------------------------------------------------------//
// Sema::LookupResult 
//----------------------------------------------------------------------------//

Sema::LookupResult::LookupResult(ResultVec&& results): results_(results) {}

void Sema::LookupResult::addResult(NamedDecl* decl) {
  results_.push_back(decl);
}

Sema::LookupResult::ResultVec& Sema::LookupResult::getResults() {
  return results_;
}

std::size_t Sema::LookupResult::size() const {
  return results_.size();
}

bool Sema::LookupResult::isEmpty() const {
  return (results_.size() == 0);
}
