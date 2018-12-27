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
#include <algorithm>

using namespace fox;

//----------------------------------------------------------------------------//
// Unqualified Lookup
//----------------------------------------------------------------------------//

namespace {
  using ResultFoundFn = std::function<bool(NamedDecl*)>;

  // Does lookup in a DeclContext.
  //  The lookup stop if:
  //    > onFound(...) returns false
  //  *or*
  //    > if we are done searching the whole DeclContext tree.
  void lookupInDeclContext(Identifier id, ResultFoundFn onFound, 
    DeclContext* dc) {
    using LMap = DeclContext::LookupMap;
    DeclContext* cur = dc;
    while(cur) {
      // DeclContext uses a std::multimap
      const LMap& map = cur->getLookupMap();
      // Search all decls with the identifier "id" in the multimap
      LMap::const_iterator beg, end;
      std::tie(beg, end) = map.equal_range(id);
      for(auto it = beg; it != end; ++it) {
        if(!onFound(it->second)) return;
      }
      // Continue climbing
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
      // Scope uses a std::map
      const auto& map = cur->getDeclsMap();
      // Search for the decl with the identifier "id" 
      // in the map
      auto it = map.find(id);
      if(it != map.end())
        if(!onFound(it->second)) return;
      // Climb parent scopes
      cur = cur->getParent();
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
  const LookupOptions& options) {
  assert((results.size() == 0) && "'results' must be a fresh LookupResult");
  assert(id && "can't lookup with invalid id!");
  bool lookInDeclCtxt = options.canLookInDeclContext;

  // Lambda that returns true if the result should be ignored.
  auto shouldIgnore = [&](NamedDecl* decl) {
    auto fn = options.shouldIgnore;
    return fn ? fn(decl) : false;
  };

  // Check in local scope, if there's one.
  if(hasLocalScope()) {
    LocalScope* scope = getLocalScope();
    // Handle results
    auto handleResult = [&](NamedDecl* decl) {
      // If we should ignore this result, do so and continue looking.
      if(shouldIgnore(decl)) return true;
      // If not, add the decl to the results and stop looking
      results.addResult(decl);
      return false;
    };
    // Do the lookup in the local scope
    lookupInLocalScope(id, handleResult, scope);
    
    // If the caller actually wanted to us to look inside
    // the DeclContext, check if it's needed. If we have found what
    // we were looking for inside the scope, there's no need to keep
    // looking.
    lookInDeclCtxt &= (results.size() == 0);
  }

  // Check in decl context if allowed to
  if(lookInDeclCtxt) {
    DeclContext* dc = getDeclCtxt();
    // We should ALWAYS have a DC, else, something's broken.
    assert(dc && "No DeclContext available?");
    // Handle results
    auto handleResult = [&](NamedDecl* decl) {
      // If we should ignore this result, do so and continue looking.
      if(shouldIgnore(decl)) return true;
      // If not, add the decl to the results and continue looking
      results.addResult(decl);
      return true;
    };
    lookupInDeclContext(id, handleResult, dc);
  }
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
