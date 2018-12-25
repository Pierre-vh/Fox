//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : Decl.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the Decl hierarchy
//----------------------------------------------------------------------------//

#pragma once

#include "llvm/ADT/PointerUnion.h"
#include "llvm/ADT/SmallVector.h"
#include "DeclContext.hpp"
#include "ASTAligns.hpp"
#include "Type.hpp"
#include "Identifier.hpp"
#include "llvm/ADT/TrailingObjects.h"
#include "llvm/ADT/ArrayRef.h"

namespace fox {
  // Forward Declarations
  class Expr;
  class Identifier;
  class ASTContext;
  class CompoundStmt;
  class FuncDecl;
  // This enum represents every possible Declaration subclass. 
  // It is automatically generated using Fox/AST/DeclNodes.def
  enum class DeclKind : std::uint8_t {
    #define DECL(ID,PARENT) ID,
    #define DECL_RANGE(ID,FIRST,LAST) First_##ID = FIRST, Last_##ID = LAST,
    #define LAST_DECL(ID) LastDecl = ID
    #include "DeclNodes.def"
  };

  inline constexpr auto toInt(DeclKind kind) {
    return static_cast<std::underlying_type<DeclKind>::type>(kind);
  }

  // Decl
  //    Common base class for every Declaration
  //    Note that every Decl will take a DeclContext* argument. That DeclContext
  //    should be their parent DeclContext.
  class alignas(DeclAlignement) Decl {
    static constexpr unsigned kindBits_ = 4;
    public:
      enum class CheckState {
        Unchecked,
        // TODO: Add "Checking". Will be needed with multi file support or variable
        //  type inference at the global level!
        Checked
        // There's room for 2 more CheckStates. If this enum is updated
        // beyond 4 elements, increase the size of it's bitfield in Decl.
      };

      using Parent = llvm::PointerUnion<DeclContext*, FuncDecl*>;

      // Get the kind of Decl this is.
      DeclKind getKind() const;

      // For normal Decls, return the DeclContext in which
      // this Decl is referenced. Returns nullptr for
      // local decls, or if the parent is null.
      DeclContext* getDeclContext() const;

      // Returns true if this is a local declaration
      bool isLocal() const;

      // For local decls, returns the FuncDecl in which
      // this declaration lives. Return nullptr for non local decls,
      // or if the parent is null.
      FuncDecl* getFuncDecl() const;

      Parent getParent() const;
      bool isParentNull() const;

      // Returns the "closest" DeclContext.
      //  -> If this Decl is also a DeclContext, returns 
      //      dyn_cast<DeclContext>(this)
      //  -> else, if this Decl is a local Decl, returns 
      //      getFuncDecl()->getDeclContext()
      //  -> Else, returns getDeclContext()
      // Should never return nullptr in a well-formed AST.
      DeclContext* getClosestDeclContext() const;

      // Sets the range of this Decl
      void setRange(SourceRange range);

      // Returns the range of this Decl, if it has one.
      SourceRange getRange() const;

      // Returns the begin loc of this Decl, if it has one.
      SourceLoc getBegin() const;

      // Returns the end loc of this Decl, if it has one.
      SourceLoc getEnd() const;

      // Return true if this Decl's CheckState is Unchecked.
      bool isUnchecked() const;
      // Return true if this Decl's CheckState is Checked
      bool isChecked() const;

      CheckState getCheckState() const;
      void markAsChecked();

      // Get the FileID of the file where this Decl is located
      FileID getFileID() const;

      // Debug method. Does a ASTDump of this node to std::cerr
      void dump() const;

    protected:
      // Operator new/delete overloads : They're protected as they should
      // only be used through ::create

      // Prohibit the use of builtin placement new & delete
      void* operator new(std::size_t) throw() = delete;
      void operator delete(void *) throw() = delete;
      void* operator new(std::size_t, void*) = delete;

      // Only allow allocation through the ASTContext
      void* operator new(std::size_t sz, ASTContext &ctxt, 
        std::uint8_t align = alignof(Decl));

      Decl(DeclKind kind, Parent parent, SourceRange range);

    private:
      Parent parent_;
      SourceRange range_;

      static_assert(toInt(DeclKind::LastDecl) < (1 << kindBits_),
        "kind_ bitfield doesn't have enough bits to represent every DeclKind");

      // Bitfield : 1 bit left
      const DeclKind kind_ : kindBits_; // The Kind of Decl this is
      CheckState checkState_ : 2; // The CheckState of this Decl
  };

  // NamedDecl
  //    Common base class for every named declaration
  //    (a declaration with an identifier)
  class NamedDecl : public Decl {
    public:
      Identifier getIdentifier() const;
      void setIdentifier(Identifier id);
      void setIdentifier(Identifier id, SourceRange idRange);
      bool hasIdentifier() const;

      bool isIllegalRedecl() const;
      void setIsIllegalRedecl(bool val);

      SourceRange getIdentifierRange() const;
      void setIdentifierRange(SourceRange range);
      bool hasIdentifierRange() const;

      static bool classof(const Decl* decl) {
        return (decl->getKind() >= DeclKind::First_NamedDecl) 
          && (decl->getKind() <= DeclKind::Last_NamedDecl);
      }

    protected:
      NamedDecl(DeclKind kind, Parent parent, Identifier id, 
        SourceRange idRange, SourceRange range);

    private:
      Identifier identifier_;
      SourceRange identifierRange_;
      // NamedDecl bitfields : 7 bit left
      bool illegalRedecl_ : 1;
  };

  // A vector of named decls
  using NamedDeclVec = SmallVector<NamedDecl*, 4>;

  // ValueDecl
  //    Common base class for every value declaration 
  //    (declares a value of a certain type & name)
  //    + a "const" attribute. 
  class ValueDecl : public NamedDecl {
    public:
      Type getType() const;
      SourceRange getTypeRange() const;
      TypeLoc getTypeLoc() const;
      void setTypeLoc(TypeLoc ty);

      bool isConst() const;
      void setIsConst(bool k);

      static bool classof(const Decl* decl) {
        return (decl->getKind() >= DeclKind::First_ValueDecl) 
          && (decl->getKind() <= DeclKind::Last_ValueDecl);
      }

    protected:
      ValueDecl(DeclKind kind, Parent parent, Identifier id, 
        SourceRange idRange, TypeLoc ty, bool isConst, SourceRange range);

    private:
      // ValueDecl bitfields : 7 left
      bool const_ : 1;
      TypeLoc type_;
  };

  // ParamDecl
  //    A declaration of a function parameter. This is simply a ValueDecl,
  //    and is constant by default.
  class ParamDecl final : public ValueDecl {
    public:
      static ParamDecl* create(ASTContext& ctxt, FuncDecl* parent, 
        Identifier id, SourceRange idRange, TypeLoc type, 
        bool isMutable, SourceRange range);

      bool isMutable() const;

      static bool classof(const Decl* decl) {
        return decl->getKind() == DeclKind::ParamDecl;
      }

    private:
      ParamDecl(FuncDecl* parent, Identifier id, SourceRange idRange, 
        TypeLoc type, bool isMutable, SourceRange range);
  };

  // ParamList
  //    Represents a list of ParamDecls
  class ParamList final : llvm::TrailingObjects<ParamList, ParamDecl*> {
    public:
      using SizeTy = std::uint8_t;
      static constexpr auto maxParams = std::numeric_limits<SizeTy>::max();

      static ParamList* create(ASTContext& ctxt, ArrayRef<ParamDecl*> params);

      ArrayRef<ParamDecl*> getArray() const;
      MutableArrayRef<ParamDecl*> getArray();
      ParamDecl*& get(std::size_t idx);
      const ParamDecl* get(std::size_t idx) const;
      SizeTy getNumParams() const;

      using iterator = MutableArrayRef<ParamDecl*>::iterator;
      using const_iterator = ArrayRef<ParamDecl*>::iterator;

      iterator begin();
      iterator end();

      const_iterator begin() const;
      const_iterator end() const;

      const ParamDecl* operator[](std::size_t idx) const;
      ParamDecl*& operator[](std::size_t idx);

    private:
      ParamList(ArrayRef<ParamDecl*> params);

      // Prohibit the use of the vanilla new/delete
      void *operator new(std::size_t) throw() = delete;
      void operator delete(void *) throw() = delete;

      // Also, allow allocation with a placement new
      // (needed for class using trailing objects)
      void* operator new(std::size_t , void* mem);

      SizeTy numParams_;
  };
  
  // FuncDecl
  //    A function declaration
  class FuncDecl final: public NamedDecl {
    public:
      static FuncDecl* create(ASTContext& ctxt, DeclContext* parent,
        Identifier id, SourceRange idRange, TypeLoc type, 
        SourceRange range, SourceLoc headerEnd);

      void setLocs(SourceRange range, SourceLoc headerEndLoc);
      void setHeaderEndLoc(SourceLoc loc);

      SourceLoc getHeaderEndLoc() const;
      SourceRange getHeaderRange() const;

      void setReturnTypeLoc(TypeLoc ty);
      TypeLoc getReturnTypeLoc() const;
      Type getReturnType() const;
      SourceRange getReturnTypeRange() const;

      void setBody(CompoundStmt* body);
      CompoundStmt* getBody() const;

      void setParams(ParamList* params);
      const ParamList* getParams() const;
      ParamList* getParams();
      bool hasParams() const;

      static bool classof(const Decl* decl) {
        return decl->getKind() == DeclKind::FuncDecl;
      }
      
    private:
      FuncDecl(DeclContext* parent, Identifier fnId, SourceRange idRange,
        TypeLoc rtrTy, SourceRange range, SourceLoc headerEndLoc);

      SourceLoc headEndLoc_;
      TypeLoc returnType_;
      ParamList* params_ = nullptr;
      CompoundStmt* body_ = nullptr;
  };

  // VarDecl
  //    A let or var variable declaration. 
  //    This is simply a ValueDecl with an added "init" Expr*. It is
  //    constant if declared using "let", non const if declared
  //    using "var".
  class VarDecl final: public ValueDecl {
    public:
      static VarDecl* create(ASTContext& ctxt, Parent parent,
        Identifier id, SourceRange idRange, TypeLoc type, 
        bool isConst, Expr* init, SourceRange range);

      Expr* getInitExpr() const;
      void setInitExpr(Expr* expr);
      bool hasInitExpr() const;

      // Returns true if this variable was declared using the "var" keyword
      bool isVar() const;
      // Returns true if this variable was declared using the "let" keyword
      bool isLet() const;

      static bool classof(const Decl* decl) {
        return decl->getKind() == DeclKind::VarDecl;
      }

    private:
      VarDecl(Parent parent, Identifier id, SourceRange idRange, TypeLoc type,
        bool isConst, Expr* init, SourceRange range);

      Expr* init_ = nullptr;
  };

  // UnitDecl
  //    Represents a parsed Source file.
  class UnitDecl final: public Decl, public DeclContext {
    public:
      static UnitDecl* create(ASTContext& ctxt, DeclContext* parent,
        Identifier id, FileID file);

      Identifier getIdentifier() const;
      void setIdentifier(Identifier id);

      // Return the ASTContext this Decl lives in.
      ASTContext& getASTContext();

      static bool classof(const Decl* decl) {
        return decl->getKind() == DeclKind::UnitDecl;
      }

      static bool classof(const DeclContext* dc) {
        return dc->getDeclContextKind() == DeclContextKind::UnitDecl;
      }

    private:
      UnitDecl(ASTContext& ctxt, DeclContext* parent, Identifier id, 
        FileID inFile);

      Identifier identifier_;
      ASTContext& ctxt_;
  };
}

