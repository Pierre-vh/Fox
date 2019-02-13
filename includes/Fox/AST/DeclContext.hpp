//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : DeclContext.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// Contains the DeclContext & derived classes.
//----------------------------------------------------------------------------//

#pragma once

#include "Identifier.hpp"
#include "ASTAligns.hpp"
#include "Fox/Common/LLVM.hpp"
#include "llvm/ADT/PointerIntPair.h"
#include <iterator>
#include <map>
#include <memory>

namespace fox {
  class Decl;
  class NamedDecl;
  class FileID;
  class ASTContext;

  enum class DeclContextKind : std::uint8_t {
    #define DECL_CTXT(ID, PARENT) ID,
    #define LAST_DECL_CTXT(ID) LastDeclCtxt = ID
    #include "DeclNodes.def"
  };

  inline constexpr auto toInt(DeclContextKind kind) {
    return static_cast<std::underlying_type<DeclContextKind>::type>(kind);
  }

  // DeclContext is a class that acts as a "semantic container for decls".
  // TODO: Add doc
  class alignas(DeclContextAlignement) DeclContext {
    public:
      // Returns the Kind of DeclContext this is
      DeclContextKind getDeclContextKind() const;

      // Return the ASTContext by walking up to the root UnitDecl
      // and returning it's ASTContext.
      ASTContext& getASTContext() const;

      bool hasParentDeclCtxt() const;
      DeclContext* getParentDeclCtxt() const;

      static bool classof(const Decl* decl);

    protected:
      DeclContext(DeclContextKind kind, DeclContext* parent = nullptr);

    private:
      // The PointerIntPair used to represent the ParentAndKind bits
      using ParentAndKindTy 
        = llvm::PointerIntPair<DeclContext*, DeclContextFreeLowBits>;
      
      // A PointerIntPair which contains the parent of this DeclContext + the
      // kind of DeclContext this is.
      const ParentAndKindTy parentAndKind_;

      // Check that ParentAndKindTy has enough bits to represent
      // every possible DeclContextKind
      static_assert(
        (1 << DeclContextFreeLowBits) > toInt(DeclContextKind::LastDeclCtxt),
        "The PointerIntPair doesn't have enough bits to represent every "
        " DeclContextKind value");
  };
  
  // DeclIterator iterates over all Decls in a DeclContext
  class DeclIterator {
    public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = Decl*;
      using difference_type = std::ptrdiff_t;

      DeclIterator() = default;
      DeclIterator(Decl* cur);

      // Pre-increment
      DeclIterator& operator++();
      // Post-increment
      DeclIterator operator++(int);

      Decl* operator*() const;
      Decl* operator->() const; 

      friend bool operator==(DeclIterator lhs, DeclIterator rhs);
      friend bool operator!=(DeclIterator lhs, DeclIterator rhs);

    private:
      Decl* cur_ = nullptr;
  };

  // DeclRange represents the range of Decls belonging to a DeclContext.
  // It provides a begin() and end() method, which enables usage in
  // for loops using the range syntax.
  class DeclRange {
    public:
      DeclRange(DeclIterator beg, DeclIterator end);

      DeclIterator begin() const;
      DeclIterator end() const;

      bool isEmpty() const;

    private:
      DeclIterator beg_, end_;
  };

  // The LookupContext is a class derived from DeclContext. It has
  // the added functionality of storing/recording Declarations and
  // enabling Lookup through a LookupMap.
  class LookupContext : public DeclContext {
    public:
      // The type of the lookup map
      using LookupMap = std::multimap<Identifier, NamedDecl*>;

      // Adds a Decl in this LookupContext.
      // If "decl" is a NamedDecl, it is expected to have a valid identifier
      void addDecl(Decl* decl);

      // Returns the (half-open) range of Decls contained in
      // this DeclContext.
      DeclRange getDecls() const;

      // Returns the first declaration of this Context.
      Decl* getFirstDecl() const;
      // Returns the last declaration of this Context.
      Decl* getLastDecl() const;

      // Returns the LookupMap
      const LookupMap& getLookupMap();

      static bool classof(const Decl* decl);
      static bool classof(const DeclContext* decl);

    protected:
      LookupContext(ASTContext& ctxt, DeclContextKind kind, 
                    DeclContext* parent = nullptr);

    private:
      friend class ASTContext; // Needs to see DeclData

      // Builds the lookup map if it's nullptr.
      void buildLookupMap();

      // The First and Last decl in the linked list of Decls
      // contained inside this DeclContext.
      Decl* firstDecl_ = nullptr;
      Decl* lastDecl_ = nullptr;

      // FIXME: Put the ASTContext in an union or something.
      ASTContext& ctxt_;
      LookupMap* lookupMap_ = nullptr;
  };
}