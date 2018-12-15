//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : DeclContext.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// Contains the DeclContext classes and related classes.
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
  class FileID;

  enum class DeclContextKind : std::uint8_t {
    #define DECL_CTXT(ID, PARENT) ID,
    #define LAST_DECL_CTXT(ID) LastDeclCtxt = ID
    #include "DeclNodes.def"
  };

  inline constexpr auto toInt(DeclContextKind kind) {
    return static_cast<std::underlying_type<DeclContextKind>::type>(kind);
  }

  // DeclContext is a class that acts as a "Declaration Recorder", which is
  // helps during semantic analysis. A DeclContext records every Declaration
  // that happens in it's children and has functions to help with Lookup. It
  // offers a Semantic view of the declarations it contains.
  //
  // It can also, on demand, generate a Lexical view of the Declarations
  // it contains.
  //
  // Keep in mind that, currently, the DeclContext has the following 
  // limitations.
  //  - It can only store non-null LOCAL NamedDecls with:
  //      - A valid Identifier object
  //      - A valid SourceRange
  //    NOTE: Assertions in addDecl guarantee that every decl added in a 
  //          DeclContext meets theses requirements.
  //  - It generates the lexical view on demand, each time it's
  //    requested. It is not cached.
  class alignas(DeclContextAlignement) DeclContext {
    public:
      //----------------------------------------------------------------------//
      // Type Aliases
      //----------------------------------------------------------------------//

      // The type of the internal map of Decls.
      using LookupMap = std::multimap<Identifier, NamedDecl*>;

      // The type of the map used to represent Decls in Lexical order.
      using LexicalDeclsTy = std::vector<NamedDecl*>;

      //----------------------------------------------------------------------//
      // Interface
      //----------------------------------------------------------------------//

      // Constructor
      //  parent may be omitted
      DeclContext(DeclContextKind kind, DeclContext* parent = nullptr);

      // Returns the Kind of DeclContext this is
      DeclContextKind getDeclContextKind() const;

      // Adds a Decl in this DeclContext
      void addDecl(NamedDecl* decl);

      // Getter for the DeclMap, which is a std::multimap. This map has
      // no particular ordering, and will be used for Lookup.
      //
      // This map is const (read only).
      const LookupMap& getDeclsMap() const;

      // Returns a vector of all Stored Decl where the decls are
      // stored in order of appearance in the file "file"
      // Use this when you want to iterate over the decls in
      // Lexical order.
      // 
      // This is pretty costly to generate so only use this when absolutely
      // necessary.
      // (and it doesn't return a reference, so be careful about copies 
      //  of the return value)
      LexicalDeclsTy getLexicalDecls(FileID forFile);

      void setParentDeclCtxt(DeclContext *dr);
      bool hasParentDeclCtxt() const;
      DeclContext* getParentDeclCtxt() const;

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
      LookupMap decls_;

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