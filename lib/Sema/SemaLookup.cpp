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
      cur = scope->getParent();
    }
  }

}

//----------------------------------------------------------------------------//
// Sema methods impl
//----------------------------------------------------------------------------//

std::pair<bool, bool> 
Sema::addToScope(NamedDecl* decl, bool canReplace) {
  if(hasLocalScope() && decl->isLocal()) {
    LocalScope* scope = getLocalScope();
    if(canReplace) {
      // if we can replace, use ->forceAdd
      bool result = scope->forceAdd(decl);
      return {true, result};
    }
    else {
      // if we can't replace, use ->add
      bool result = scope->add(decl);
      return {result, true};
    }
  }
  return {false, false};
}

void Sema::doUnqualifiedLookup(LookupResult& results, Identifier id,
  bool lookInDeclCtxt) {
  assert((results.size() == 0) && "'results' must be a fresh LookupResult");
  // Check in local scope, if there's one.
  if(hasLocalScope()) {
    LocalScope* scope = getLocalScope();
    // Handle results
    auto handleResult = [&](NamedDecl* decl) {
      // Add the decl and stop looking
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
      // Add the decl and continue looking
      results.addResult(decl);
      return true;
    };
    lookupInDeclContext(id, handleResult, dc);
  }

  // Set the kind of LookupResult
  using LRK = LookupResult::Kind;
  if(results.size() == 0)
    results.setKind(LRK::NotFound);
  else if(results.size() == 1)
    results.setKind(LRK::Found);
  else 
    results.setKind(LRK::Ambiguous);
}

//----------------------------------------------------------------------------//
// Sema::LookupResult 
//----------------------------------------------------------------------------//

Sema::LookupResult::LookupResult(Kind kind, ResultVec&& results): kind_(kind),
  results_(results) {}

void Sema::LookupResult::addResult(NamedDecl* decl) {
  results_.push_back(decl);
}

Sema::LookupResult::ResultVec& Sema::LookupResult::getResults() {
  return results_;
}

std::size_t Sema::LookupResult::size() const {
  return results_.size();
}

Sema::LookupResult::Kind Sema::LookupResult::getKind() const {
  return kind_;
}

void Sema::LookupResult::setKind(Kind kind) {
  kind_ = kind;
}

NamedDecl* Sema::LookupResult::getIfSingleResult() const {
  if(kind_ != Kind::Found) return nullptr;
  // For "Found" kind, we should only have 1 result.
  assert(results_.size() == 1);
  return results_[0];
}

bool Sema::LookupResult::isNotFound() const {
  return (kind_ == Kind::NotFound);
}

bool Sema::LookupResult::isFound() const {
  return (kind_ == Kind::Found);
}

bool Sema::LookupResult::isAmbiguous() const {
  return (kind_ == Kind::Ambiguous);
}
