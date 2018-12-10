//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : DeclContext.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// DeclContext is a class that acts as a "Declaration Recorder", which is
// helps during semantic analysis. A DeclContext records every Declaration
// that happens in it's children and has functions to help with Lookup.
//----------------------------------------------------------------------------//

// TODO: Abstract the "DeclsMap" better. Maybe create a wrapper
// around the std::multimap so I can change the implementation to something
// more efficient without breaking the interface.

#pragma once

#include "Identifier.hpp"
#include "llvm/ADT/PointerIntPair.h"
#include "ASTAligns.hpp"
#include <map>
#include <vector>
#include <type_traits>

namespace fox {

  class Decl;
  class NamedDecl;
  class LookupResult;

  // An iterator that abstracts the underlying structure 
  // (an iterator to std::pair) used by the DeclContext 
  // to only show the NamedDecl pointer to the client.
  template <typename BaseIterator>
  class DeclContextIterator : public BaseIterator {
    public:
      using value_type = typename BaseIterator::value_type::second_type;

      DeclContextIterator(const BaseIterator &baseIt) : BaseIterator(baseIt) {
        static_assert(std::is_same<value_type, NamedDecl*>::value,
					"Pointer type isn't a NamedDecl*");
      }

      // Operator * returns the pointer
      value_type operator*() const { 
        return (this->BaseIterator::operator*()).second; 
      }
      // Operator -> lets you access the members directly. It's equivalent to (*it)->
      value_type operator->() const { 
        return (this->BaseIterator::operator*()).second; 
      }
  };

  enum class DeclContextKind : std::uint8_t {
    #define DECL_CTXT(ID, PARENT) ID,
    #define LAST_DECL_CTXT(ID) LastDeclCtxt = ID
    #include "DeclNodes.def"
  };

  static constexpr auto toInt(DeclContextKind kind) {
    return static_cast<std::underlying_type<DeclContextKind>::type>(kind);
  }

  class alignas(DeclContextAlignement) DeclContext {
    private:
      using DeclsMapTy = std::multimap<Identifier, NamedDecl*>;
      using DeclMapIter 
        = DeclContextIterator<DeclsMapTy::iterator>;
      using DeclMapConstIter 
        = DeclContextIterator<DeclsMapTy::const_iterator>;

    public:
      // \param kind the Kind of DeclContext this is
      DeclContext(DeclContextKind kind, DeclContext* parent = nullptr);

      DeclContextKind getDeclContextKind() const;

      // Record (adds) a declaration within this DeclContext
      void recordDecl(NamedDecl* decl);

      // Returns true if this is a local context.
      bool isLocalDeclContext() const;

      // Getter for the DeclMap, which will be used to do the Lookup
      DeclsMapTy& getDeclsMap();

      // Manage parent decl recorder
      bool hasParent() const;
      DeclContext* getParent() const;
      void setParent(DeclContext *dr);

      // Get information
      std::size_t numDecls()  const;

      DeclMapIter decls_begin();
      DeclMapIter decls_end();

      DeclMapConstIter decls_begin() const;
      DeclMapConstIter decls_end() const;

      static bool classof(const Decl* decl);

    private:
      // The PointerIntPair used to represent the ParentAndKind bits
      using ParentAndKindTy 
        = llvm::PointerIntPair<DeclContext*, DeclContextFreeLowBits>;
      // Check that ParentAndKindTy has enough bits to represent
      // every possible DeclContextKind
      static_assert(
        (1 << DeclContextFreeLowBits) > toInt(DeclContextKind::LastDeclCtxt),
        "The PointerIntPair doesn't have enough bits to represent every "
        " DeclContextKind value");
      ParentAndKindTy parentAndKind_;
      DeclsMapTy namedDecls_;
  };
}