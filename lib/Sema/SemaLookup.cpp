//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
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
#include <algorithm>

using namespace fox;

//----------------------------------------------------------------------------//
// Unqualified Lookup
//----------------------------------------------------------------------------//

namespace {
  using ResultFoundFn = std::function<bool(NamedDecl*)>;

  // Does lookup in a LocalScope.
  //   > The lookup stop if onFound(...) returns false
  // *or*
  //   > if we are done searching the whole scope tree.
  void lookupInLocalScope(Identifier id, ResultFoundFn onFound, 
    LocalScope* scope) {
    LocalScope* cur = scope;
    while(cur) {
      const auto& map = cur->getDeclsMap();
      // Try to find a decl with the identifier "id" in the multimap
      auto it = map.find(id);
      if(it != map.end())
        if(!onFound(it->second)) return;
      // Climb parent scopes
      cur = cur->getParentIfLocalScope();
    }
  }
}

//----------------------------------------------------------------------------//
// Sema methods impl
//----------------------------------------------------------------------------//

std::pair<bool, bool>  Sema::addLocalDeclToScope(NamedDecl* decl) {
  assert(decl->isLocal() && "This method is only available to local decls");
  if(hasLocalScope()) {
    bool result = getLocalScope()->insert(decl);
    return {true, result};
  }
  return {false, false};
}

void Sema::doUnqualifiedLookup(LookupResult& results, Identifier id,
  SourceLoc loc, const LookupOptions& options) {
  assert((results.size() == 0) && "'results' must be a fresh LookupResult");
  assert(id && "can't lookup with invalid id!");

  // If this is set to false, we don't look inside the DeclContexts and 
  // we limit the search to LocalScopes.
  bool lookInDeclCtxt = options.canLookInDeclContext;

  // If we find a VarDecl that's currently being checked, it's ignored and
  // stored in "checkingVar". If we finish lookup and we still find nothing,
  // we return checkingVar.
  //
  // This is needed to allow cases such as
  //  func foo(x : int) {
  //    var x : int = x; // x binds to the Parameter, not the variable.
  //  }
  // 
  // And checkingVar is still returned when nothing is found so cases such as
  //  let x : int = x;
  // can still be diagnosed correctly.

  NamedDecl* checkingVar = nullptr;

  // Helper lambda that returns true if a lookup result should be ignored.
  auto shouldIgnore = [&](NamedDecl* decl) {
    auto fn = options.shouldIgnore;
    return fn ? fn(decl) : false;
  };

  // Check in the local scope, if there's one.
  if(hasLocalScope()) {
    LocalScope* scope = getLocalScope();
    // Helper lambda that handles results.
    auto handleResult = [&](NamedDecl* decl) {
      // If we should ignore this result, do so 
      if(shouldIgnore(decl)) return true;
      // If the decl is VarDecl that's currently being checked, and that
      // happens in a LocalScope, don't push it to the results just yet.
      if(isa<VarDecl>(decl) && decl->isChecking()) {
        // Normally only 1 variable should be in the "checking" state.
        assert(!checkingVar && "more than 1 variable in the Checking state");
        checkingVar = decl;
        // Keep looking 
        return true;
      }
      // In local scopes, we stop on the first result found.
      results.addResult(decl);
      return false;
    };
    // Do the lookup in the local scope
    lookupInLocalScope(id, handleResult, scope);
    
    // If the caller actually wanted to us to look inside
    // the DeclContext, check if it's still needed. If we have found what
    // we were looking for inside the scope, there's no need to look
    // in the DeclContext.
    lookInDeclCtxt &= (results.size() == 0);
  }

  // Check in decl context if allowed to
  if(lookInDeclCtxt) {
    DeclContext* currentDeclContext = getDeclCtxt();
    // We should ALWAYS have an active DC, else something's broken.
    assert(currentDeclContext 
      && "No DeclContext available?");
    // Handle results
    auto handleResult = [&](NamedDecl* decl) {
      // If we should ignore this result, do so and continue looking.
      if(shouldIgnore(decl)) return true;

      // If not, add the decl to the results and continue looking
      results.addResult(decl);
      return true;
    };
    // Do the lookup 
    {
      // We're going to iterate over each parent in the DeclContext
      // hierarchy, but we'll stop once handleResult returns false.
      while (currentDeclContext) {
        // TODO: Remove this check once DeclContext supports local
        // lookup.
        if (!currentDeclContext->isLocal()) {
          // The SourceLoc only matters when looking inside the currently
          // active DeclContext.
          SourceLoc theLoc = 
            (currentDeclContext == getDeclCtxt()) ? loc : SourceLoc();
          if(!currentDeclContext->lookup(id, theLoc, handleResult))
            break;
        }
        // Climb
        currentDeclContext = currentDeclContext->getParentDeclCtxt();
      }
    }
  }

  // Add the checkingVar if the result set is empty.
  if(results.isEmpty() && checkingVar)
    results.addResult(checkingVar);
}

//----------------------------------------------------------------------------//
// Sema::LookupResult 
//----------------------------------------------------------------------------//

void Sema::LookupResult::addResult(NamedDecl* decl) {
  results_.push_back(decl);
}

NamedDeclVec& Sema::LookupResult::getResults() {
  return results_;
}

const NamedDeclVec& Sema::LookupResult::getResults() const {
  return results_;
}

std::size_t Sema::LookupResult::size() const {
  return results_.size();
}

NamedDecl* Sema::LookupResult::getIfSingleResult() const {
  if(results_.size() == 1)
    return results_[0];
  return nullptr;
}

bool Sema::LookupResult::isEmpty() const {
  return (size() == 0);
}

bool Sema::LookupResult::isAmbiguous() const {
  return (size() > 1);
}

NamedDeclVec::iterator Sema::LookupResult::begin() {
  return results_.begin();
}

NamedDeclVec::const_iterator Sema::LookupResult::begin() const {
  return results_.begin();
}

NamedDeclVec::iterator Sema::LookupResult::end() {
  return results_.end();
}

NamedDeclVec::const_iterator Sema::LookupResult::end() const {
  return results_.end();
}
