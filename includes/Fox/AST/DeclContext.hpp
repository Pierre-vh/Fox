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

  class alignas(align::DeclContextAlignement) DeclContext {
    private:
      using NamedDeclsMapTy = std::multimap<Identifier, NamedDecl*>;
      using NamedDeclsMapIter 
        = DeclContextIterator<NamedDeclsMapTy::iterator>;
      using NamedDeclsMapConstIter 
        = DeclContextIterator<NamedDeclsMapTy::const_iterator>;

    public:
      // \param kind the Kind of DeclContext this is
      DeclContext(DeclContextKind kind, DeclContext* parent = nullptr);

      DeclContextKind getDeclContextKind() const;

      // "Record" a declaration within this DeclContext
      void recordDecl(NamedDecl* decl);

      // Returns true if this is a local context.
      bool isLocalDeclContext() const;

      // Searches for every NamedDecl with the Identifier id 
			// in this DeclContext
      LookupResult restrictedLookup(Identifier id) const;

      // Performs a full lookup. Searches this DeclContext
			// as well as parent ones.
      LookupResult fullLookup(Identifier id) const;

      // Manage parent decl recorder
      bool hasParent() const;
      DeclContext* getParent() const;
      void setParent(DeclContext *dr);

      // Get information
      std::size_t getNumberOfRecordedDecls()  const;

      NamedDeclsMapIter recordedDecls_begin();
      NamedDeclsMapIter recordedDecls_end();

      NamedDeclsMapConstIter recordedDecls_begin() const;
      NamedDeclsMapConstIter recordedDecls_end() const;

      static bool classof(const Decl* decl);

    private:
      // Helpers
      static constexpr unsigned parentAndKindBits = 1;
      static constexpr auto lastDeclCtxtValue 
        = toInt(DeclContextKind::LastDeclCtxt);
      // The PointerIntPair used to represent the ParentAndKind bits
      using ParentAndKindTy 
        = llvm::PointerIntPair<DeclContext*, parentAndKindBits>;
      // Check that ParentAndKindTy has enough bits to represent
      // every possible DeclContextKind
      static_assert(
        (1 << parentAndKindBits) > lastDeclCtxtValue,
        "The PointerIntPair doesn't have enough bits to represent every "
        " DeclContextKind value");
      ParentAndKindTy parentAndKind_;
      NamedDeclsMapTy namedDecls_;
  };

  class LookupResult {
    private:
      using ResultVecTy = std::vector<NamedDecl*>;
      using ResultVecIter = ResultVecTy::iterator;
      using ResultVecConstIter = ResultVecTy::const_iterator;

    public:
      LookupResult();

      std::size_t size() const;

      ResultVecIter begin();
      ResultVecConstIter begin() const;

      ResultVecIter end();
      ResultVecConstIter end() const;

      // Returns true if the size() > 0
      explicit operator bool() const;

    protected:
      friend class DeclContext;

      // Add another result
      void addResult(NamedDecl* decl);

      // If the target contains at least 1 result,
      // copies all of the results from target into this l
      // ookupresult then clears the target.
      void absorb(LookupResult &target);

    private:
      ResultVecTy results_;
  };
}