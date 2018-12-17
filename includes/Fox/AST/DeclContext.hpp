//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : DeclContext.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// Contains the DeclContext class and the DeclContextKind enum
//----------------------------------------------------------------------------//

#pragma once

#include "Identifier.hpp"
#include "llvm/ADT/PointerIntPair.h"
#include "ASTAligns.hpp"
#include <map>
#include <vector>
#include <type_traits>
#include <memory>

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
  // helps during semantic analysis. 
  //
  // It'll store declarations in a vector, in order of addition. It can also
  // generate (lazily/on demand) a lookup map, which can be used to retrieve a
  // list of NamedDecl* with a given identifier.
  class alignas(DeclContextAlignement) DeclContext {
    public:
      //----------------------------------------------------------------------//
      // Type Aliases
      //----------------------------------------------------------------------//

      // The type of the lookup map
      using LookupMap = std::multimap<Identifier, NamedDecl*>;

      // The type of the vector used to store the declarations
      using DeclVec = std::vector<Decl*>;

      //----------------------------------------------------------------------//
      // Interface
      //----------------------------------------------------------------------//

      // Constructor
      //  parent may be omitted
      DeclContext(DeclContextKind kind, DeclContext* parent = nullptr);

      // Returns the Kind of DeclContext this is
      DeclContextKind getDeclContextKind() const;

      // Adds a Decl in this DeclContext.
      // If "decl" is a NamedDecl, it is expected to have a valid identifier
      void addDecl(Decl* decl);

      // Returns the vector of decls used internally by this DeclContext;
      // This is a lexically accurate view since the declarations are in order
      // of insertion (First element of the vector is the first decl added in
      // this DeclContext, the 2nd element is the 2nd decl added, and so on..)
      const DeclVec& getDecls() const;

      const LookupMap& getLookupMap();

      void setParentDeclCtxt(DeclContext *dr);
      bool hasParentDeclCtxt() const;
      DeclContext* getParentDeclCtxt() const;

      // Get the number of decls in this DeclContext
      std::size_t numDecls()  const;

      static bool classof(const Decl* decl);

    private:
      //----------------------------------------------------------------------//
      // Private methods
      //----------------------------------------------------------------------//

      // If the LookupMap has not been built yet, builds it from "decls_"
      void buildLookupMap();

      //----------------------------------------------------------------------//
      // Private members
      //----------------------------------------------------------------------//

      // The PointerIntPair used to represent the ParentAndKind bits
      using ParentAndKindTy 
        = llvm::PointerIntPair<DeclContext*, DeclContextFreeLowBits>;
      
      // A PointerIntPair which contains the parent of this DeclContext + the
      // kind of DeclContext this is.
      ParentAndKindTy parentAndKind_;
      // The vector of declarations
      DeclVec decls_;
      // The lazily generated lookup map
      std::unique_ptr<LookupMap> lookup_;

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