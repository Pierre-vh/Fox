//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : DeclContext.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// DeclContext is a class that acts as a "Declaration Recorder", which is
// helps during semantic analysis. A DeclContext records every Declaration
// that happens in it's children and has functions to help with Lookup. It
// offers a Semantic view of the declarations it contains.
//----------------------------------------------------------------------------//

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

  inline constexpr auto toInt(DeclContextKind kind) {
    return static_cast<std::underlying_type<DeclContextKind>::type>(kind);
  }

  class alignas(DeclContextAlignement) DeclContext {
    public:
      //----------------------------------------------------------------------//
      // Type Aliases
      //----------------------------------------------------------------------//

      // The type of the internal map of Decls
      using DeclsMapTy = std::multimap<Identifier, Decl*>;

      // The type of the non-const iterator for DeclsMapTy
      using DeclMapIter 
        = DeclContextIterator<DeclsMapTy::iterator>;

      // The type of the const iterator for DeclsMapTy
      using DeclMapConstIter 
        = DeclContextIterator<DeclsMapTy::const_iterator>;

      //----------------------------------------------------------------------//
      // Interface
      //----------------------------------------------------------------------//

      // Constructor
      //  parent may be omitted
      DeclContext(DeclContextKind kind, DeclContext* parent = nullptr);

      // Returns the Kind of DeclContext this is
      DeclContextKind getDeclContextKind() const;

      // Adds a Decl in this DeclContext
      void addDecl(Decl* decl);

      // Returns true if this is a local context
      bool isLocalDeclContext() const;

      // Getter for the DeclMap, which is a std::multimap.
      DeclsMapTy& getDeclsMap();

      void setParent(DeclContext *dr);
      bool hasParent() const;
      DeclContext* getParent() const;

      // Get the number of decls in this DeclContext
      std::size_t numDecls()  const;

      static bool classof(const Decl* decl);

    private:
      //----------------------------------------------------------------------//
      // Private members
      //----------------------------------------------------------------------//

      // The PointerIntPair used to represent the ParentAndKind bits
      using ParentAndKindTy 
        = llvm::PointerIntPair<DeclContext*, DeclContextFreeLowBits>;
      // Members
      ParentAndKindTy parentAndKind_;
      DeclsMapTy decls_;

      //----------------------------------------------------------------------//
      // Static assertion
      //----------------------------------------------------------------------//
      // Check that ParentAndKindTy has enough bits to represent
      // every possible DeclContextKind
      static_assert(
        (1 << DeclContextFreeLowBits) > toInt(DeclContextKind::LastDeclCtxt),
        "The PointerIntPair doesn't have enough bits to represent every "
        " DeclContextKind value");
  };
}