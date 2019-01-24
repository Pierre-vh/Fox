//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : DeclContext.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// Contains the DeclContext & derived classes.
//----------------------------------------------------------------------------//

#pragma once

#include "Identifier.hpp"
#include "ASTAligns.hpp"
#include "Fox/Common/LLVM.hpp"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/PointerIntPair.h"
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

  // The LookupContext is a class derived from DeclContext. It has
  // the added functionality of storing/recording Declarations and
  // enabling Lookup through a LookupMap.
  class LookupContext : public DeclContext {
    public:
      // The type of the lookup map
      using LookupMap = std::multimap<Identifier, NamedDecl*>;

      // The type of the vector used to store the declarations
      using DeclVec = SmallVector<Decl*, 4>;

      // Adds a Decl in this LookupContext.
      // If "decl" is a NamedDecl, it is expected to have a valid identifier
      void addDecl(Decl* decl);

      // Returns the vector of decls used internally by this DeclContext;
      // This is a lexically accurate view since the declarations are in order
      // of insertion (First element of the vector is the first decl added in
      // this DeclContext, the 2nd element is the 2nd decl added, and so on..)
      const DeclVec& getDecls() const;

      const LookupMap& getLookupMap();

      // Get the number of decls in this DeclContext
      std::size_t numDecls()  const;

      static bool classof(const Decl* decl);
      static bool classof(const DeclContext* decl);

    protected:
      LookupContext(ASTContext& ctxt, DeclContextKind kind, 
                    DeclContext* parent = nullptr);

    private:
      friend class ASTContext; // Needs to see DeclData
      struct DeclData;

      DeclData& data();
      const DeclData& data() const;

      // If the LookupMap has not been built yet, builds it from "data().decls_"
      void buildLookupMap();

      DeclData* data_ = nullptr;
  };

  // Contains the non trivially destructible objects that the
  // DeclContext needs
  struct LookupContext::DeclData {
    // Creates a DeclData allocated inside the AST
    static DeclData* create(ASTContext& ctxt, LookupContext* lc);
    // Pointer to the DeclContext that this data belongs to
    LookupContext* lc;
    // The vector of declarations
    DeclVec decls;
    // The lazily generated lookup map
    std::unique_ptr<LookupMap> lookupMap;
    private:
      DeclData(LookupContext* me) : lc(me) {}

      // Prohibit the use of builtin placement new & delete
      void* operator new(std::size_t) throw() = delete;
      void operator delete(void *) throw() = delete;
      void* operator new(std::size_t, void*) = delete;

      // Only allow allocation through the ASTContext
      void* operator new(std::size_t sz, ASTContext &ctxt, 
        std::uint8_t align = alignof(DeclData));
  };
}