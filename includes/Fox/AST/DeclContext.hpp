//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : DeclContext.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// Contains the DeclContext class
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
  class SourceLoc;
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

  // DeclContext is a class that acts as a "semantic container for declarations"
  // It tracks the declaration it "owns", and provides lookup methods.
  //
  // This class is the centerpiece of name resolution in Fox. It is used to handle
  // any kind of lookup, both Unqualified and Qualified.
  class alignas(DeclContextAlignement) DeclContext {
    // The type of the lookup map
    using LookupMap = std::multimap<Identifier, NamedDecl*>;
    public:
      // Returns the Kind of DeclContext this is
      DeclContextKind getDeclContextKind() const;

      // Return the ASTContext by walking up to the root UnitDecl
      // and returning it's ASTContext.
      ASTContext& getASTContext() const;

      bool hasParentDeclCtxt() const;
      DeclContext* getParentDeclCtxt() const;

      // Returns true if this DeclContext is a local DeclContext.
      bool isLocal() const;

      // Adds a Decl in this DeclContext.
      // If "decl" is a NamedDecl, it is expected to have a valid identifier
      void addDecl(Decl* decl);

      // Returns the (half-open) range of Decls contained in
      // this DeclContext.
      DeclRange getDecls() const;

      // Returns the first declaration of this Context.
      Decl* getFirstDecl() const;
      // Returns the last declaration of this Context.
      Decl* getLastDecl() const;

      using ResultFoundCallback = std::function<bool(NamedDecl*)>;

      // Performs a lookup in this DeclContext.
      // If loc is null, the SourceLoc is ignored and every
      // result is returned, no matter the loc.
      //
      // Note that this only looks in this DeclContext, and does
      // no climb parent DeclContexts.
      //
      // Returns true by default, false if the lookup was
      // aborted due to onFound returning false.
      // TODO: Improve doc
      bool lookup(Identifier id, SourceLoc loc, 
                  ResultFoundCallback onFound);

      static bool classof(const Decl* decl);

    protected:
      DeclContext(ASTContext& ctxt, DeclContextKind kind, 
                  DeclContext* parent);

    private:
      // The PointerIntPair used to represent the ParentAndKind bits
      using ParentAndKindTy 
        = llvm::PointerIntPair<DeclContext*, DeclContextFreeLowBits>;
      
      // A PointerIntPair which contains the parent of this DeclContext + the
      // kind of DeclContext this is.
      const ParentAndKindTy parentAndKind_;

      // The First and Last decl in the linked list of Decls
      // contained inside this DeclContext.
      Decl* firstDecl_ = nullptr;
      Decl* lastDecl_ = nullptr;

      // The lookup map
      LookupMap* lookupMap_ = nullptr;

      // Check that ParentAndKindTy has enough bits to represent
      // every possible DeclContextKind
      static_assert(
        (1 << DeclContextFreeLowBits) > toInt(DeclContextKind::LastDeclCtxt),
        "The PointerIntPair doesn't have enough bits to represent every "
        " DeclContextKind value");
  };
}