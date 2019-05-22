//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : Decl.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the Decl hierarchy
//----------------------------------------------------------------------------//

#pragma once

#include "DeclContext.hpp"
#include "ASTAligns.hpp"
#include "Type.hpp"
#include "Identifier.hpp"
#include "Fox/Common/BuiltinID.hpp"
#include "llvm/Support/TrailingObjects.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/PointerUnion.h"
#include "llvm/ADT/SmallVector.h"

namespace fox {
  // Forward Declarations
  class Expr;
  class Identifier;
  class ASTContext;
  class CompoundStmt;
  class FuncDecl;
  class UnitDecl;

  /// Enum representing every kind of declaration that exist.
  enum class DeclKind : std::uint8_t {
    #define DECL(ID,PARENT) ID,
    #define DECL_RANGE(ID,FIRST,LAST) First_##ID = FIRST, Last_##ID = LAST,
    #define LAST_DECL(ID) LastDecl = ID
    #include "DeclNodes.def"
  };

  inline constexpr auto toInt(DeclKind kind) {
    return static_cast<std::underlying_type<DeclKind>::type>(kind);
  }

  /// Decl
  ///    Common base class for every Declaration
  ///    Note that every Decl will take a DeclContext* argument. That DeclContext
  ///    should be their parent DeclContext.
  class alignas(DeclAlignement) Decl {
    Decl(const Decl&) = delete;
    Decl& operator=(const Decl&) = delete;

    public:
      /// The semantic analysis state for a Decl
      enum class CheckState : std::uint8_t {
        /// The decl has not been checked yet.
        Unchecked,

        /// The Decl is currently being checked
        Checking,

        /// The Decl has been checked
        Checked 
        // There's room for 1 more CheckStates. If this enum is updated
        // beyond 4 elements, increase the size of its bitfield.
      };

      /// \returns the kind of Decl this is.
      DeclKind getKind() const;

      /// Return the DeclContext in which this Decl is referenced.
      /// Returns nullptr for local decls, or if the parent is null.
      DeclContext* getDeclContext() const;

      /// Returns true if this is a local declaration
      bool isLocal() const;

      /// Returns the "closest" DeclContext.
      ///  -> If this Decl is also a DeclContext, returns 
      ///      dyn_cast<DeclContext>(this)
      ///  -> else, if this Decl is a local Decl, returns 
      ///      getFuncDecl()->getDeclContext()
      ///  -> Else, returns getDeclContext()
      /// Should never return nullptr in a well-formed AST.
      LLVM_ATTRIBUTE_RETURNS_NONNULL
      DeclContext* getClosestDeclContext() const;

      /// Returns the range of this Decl, if it has one.
      SourceRange getSourceRange() const;

      /// Returns the begin loc of this Decl, if it has one.
      SourceLoc getBeginLoc() const;

      /// Returns the end loc of this Decl, if it has one.
      SourceLoc getEndLoc() const;

      bool isUnchecked() const;
      bool isChecking() const;
      bool isChecked() const;

      CheckState getCheckState() const;
      void setCheckState(CheckState state);

      /// Get the FileID of the file where this Decl is located
      FileID getFileID() const;

      /// Debug method. Does a ASTDump of this node to std::cerr
      void dump() const;

      // Prohibit the use of builtin placement new & delete
      void* operator new(std::size_t) noexcept = delete;
      void operator delete(void *) noexcept = delete;
      void* operator new(std::size_t, void*) = delete;

    protected:
      // Only allow allocation through the ASTContext
      void* operator new(std::size_t sz, ASTContext& ctxt,
        std::uint8_t align = alignof(Decl));

      Decl(DeclKind kind, DeclContext* dc);

    private:
      friend class DeclContext;
      friend class DeclIterator;

      // Our parent DeclContext
      DeclContext* dc_ = nullptr;

      // The next decl in this DeclContext
      Decl* nextDecl_ = nullptr;

      static constexpr unsigned kindBits_ = 4;
      static_assert(toInt(DeclKind::LastDecl) < (1 << kindBits_),
        "kind_ bitfield doesn't have enough bits to represent every DeclKind");

      // Bitfield : 2 bit left
      const DeclKind kind_ : kindBits_; // The Kind of Decl this is
      CheckState checkState_ : 2;       // The CheckState of this Decl
  };

  /// NamedDecl
  ///    Common base class for every named declaration
  ///    (a declaration with an identifier)
  class NamedDecl : public Decl {
    public:
      Identifier getIdentifier() const;
      bool hasIdentifier() const;

      bool isIllegalRedecl() const;
      void setIsIllegalRedecl(bool val);

      SourceRange getIdentifierRange() const;
      bool hasIdentifierRange() const;

      static bool classof(const Decl* decl) {
        return (decl->getKind() >= DeclKind::First_NamedDecl) 
          && (decl->getKind() <= DeclKind::Last_NamedDecl);
      }

    protected:
      NamedDecl(DeclKind kind, DeclContext* dc, Identifier id, 
        SourceRange idRange);

    private:
      Identifier identifier_;
      SourceRange identifierRange_;
      // NamedDecl bitfields : 7 bit left
      bool illegalRedecl_ : 1;
  };

  /// A vector of named decls
  using NamedDeclVec = SmallVector<NamedDecl*, 4>;

  /// ValueDecl
  ///    Common base class for every value declaration 
  ///    (declares a value of a certain type & name)
  ///    + a "const" attribute. 
  class ValueDecl : public NamedDecl {
    public:
      Type getValueType() const;

      bool isConst() const;

      static bool classof(const Decl* decl) {
        return (decl->getKind() >= DeclKind::First_ValueDecl) 
          && (decl->getKind() <= DeclKind::Last_ValueDecl);
      }

    protected:
      ValueDecl(DeclKind kind, DeclContext* dc, Identifier id, 
        SourceRange idRange);
  };

  /// ParamDecl
  ///    A declaration of a function parameter. This is simply a ValueDecl,
  ///    and is constant by default.
  class ParamDecl final : public ValueDecl {
    public:
      static ParamDecl* create(ASTContext& ctxt, DeclContext* dc, 
        Identifier id, SourceRange idRange, TypeLoc type,
        bool isMut);

      /// \returns true if this parameter was declared with 'mut'
      bool isMut() const;

      TypeLoc getTypeLoc() const;

      /// \returns the 'value type' of this ParamDecl, which is simply the type
      /// as written down by the user. This does not include the 'mut' qualifier
      /// if present.
      Type getValueType() const;

      /// Marks this parameter as being used at least once
      void setIsUsed(bool value = true);
      /// \returns true if this parameter is used at least once
      bool isUsed() const;

      SourceRange getSourceRange() const;

      static bool classof(const Decl* decl) {
        return decl->getKind() == DeclKind::ParamDecl;
      }

    private:
      ParamDecl(DeclContext* dc, Identifier id, SourceRange idRange, 
        TypeLoc type, bool isMut);

      // ParamDecl BitField: 2 free bits left
      // FIXME: Maybe store the type and typeloc separately so we
      // can compress the bools into the Type pointer?
      const bool isMut_ : 1;
      bool used_ : 1;
      TypeLoc typeLoc_;
  };

  /// ParamList
  ///    Represents a list of ParamDecls
  ///    
  ///    ParamList does not allow the pointer values to be changed,
  ///    but the decls themselves can be altered. This means that you
  ///    can't do something like
  ///         paramList[0] = someOtherDecl
  ///    but you can do
  ///         paramList[0]->setSomething(...)
  ///
  ///    TL;DR: it feels like an ArrayRef<ParamDecl*>
  class ParamList final : llvm::TrailingObjects<ParamList, ParamDecl*> {
    using TrailingObjects = llvm::TrailingObjects<ParamList, ParamDecl*>;
    friend TrailingObjects;
    public:
      static ParamList* create(ASTContext& ctxt, ArrayRef<ParamDecl*> params);

      ArrayRef<ParamDecl*> getArray() const;
      ParamDecl* get(std::size_t idx) const;
      std::size_t size() const;

      using iterator = ArrayRef<ParamDecl*>::iterator;

      iterator begin() const;
      iterator end() const;

      ParamDecl* operator[](std::size_t idx) const;

      // Prohibit the use of the vanilla new/delete
      void *operator new(std::size_t) noexcept =  delete;
      void operator delete(void *) noexcept = delete;

    private:
      ParamList(ArrayRef<ParamDecl*> params);

      // Also, allow allocation with a placement new
      // (needed for class using trailing objects)
      void* operator new(std::size_t , void* mem);

      const std::size_t numParams_ = 0;
  };

  /// FuncDecl
  ///    A function declaration
  class FuncDecl final: public DeclContext, public ValueDecl {
    public:
      static FuncDecl* create(ASTContext& ctxt, DeclContext* parent,
        SourceLoc fnBegLoc, Identifier id, SourceRange idRange);

      /// Sets the return type of this FuncDecl. 
      /// This will nullify the ValueDecl type.
      void setReturnTypeLoc(TypeLoc ty);

      // Choose to use the DeclContext version of getASTContext() to
      // avoid ambiguities
      using DeclContext::getASTContext;

      TypeLoc getReturnTypeLoc() const;
      bool isReturnTypeImplicit() const;

      void setBody(CompoundStmt* body);
      CompoundStmt* getBody() const;

      /// Sets the parameters of this FuncDecl. 
      /// This will nullify the ValueDecl type.
      void setParams(ParamList* params);
      /// \returns the parameter list of this function, can be nullptr
      ///          if there are no parameters.
      ParamList* getParams() const;
      /// \returns true if this function has a ParamList.
      bool hasParams() const;
      /// \returns the number of parameters this function has
      std::size_t numParams() const;
      /// \returns the number of parameter this function has that are
      ///   actually used.
      std::size_t numUsedParams() const;

      /// (Re)calculates the ValueDecl type for this FuncDecl
      void calculateValueType();
      /// \returns the FunctionType for this function
      Type getValueType() const;

      SourceRange getSourceRange() const;

      static bool classof(const Decl* decl) {
        return decl->getKind() == DeclKind::FuncDecl;
      }

      static bool classof(const DeclContext* dc) {
        return dc->getDeclContextKind() == DeclContextKind::FuncDecl;
      }
      
    private:
      FuncDecl(DeclContext* parent, SourceLoc fnBegLoc, Identifier fnId,
        SourceRange idRange);
      
      Type valueType_;
      SourceLoc fnBegLoc_;
      TypeLoc returnTypeLoc_;
      ParamList* params_ = nullptr;
      CompoundStmt* body_ = nullptr;
  };
    
  /// BuiltinFuncDecl
  ///    A builtin function 'declaration'. This isn't declared
  ///    by the user, but is used to allow builtin function to
  ///    naturally mix with other Fox declarations.
  ///
  ///    They also don't have a parent DeclContext.
  class BuiltinFuncDecl final : public ValueDecl {
    public:
      /// \returns the unique BuiltinFuncDecl for the Builtin with id \p id
      /// in the context \p ctxt
      static BuiltinFuncDecl* get(ASTContext& ctxt, BuiltinID id);

      /// \returns the invalid SourceRange (SourceRange())
      SourceRange getSourceRange() const {
        return SourceRange();
      }

      /// \returns the FunctionType of this builtin
      Type getValueType() const {
        return type_;
      }

      /// \returns the BuiltinID of this BuiltinFuncDecl
      BuiltinID getBuiltinID() const {
        return bID_;
      }

      static bool classof(const Decl* decl) {
        return decl->getKind() == DeclKind::BuiltinFuncDecl;
      }

    private:
      friend ASTContext;

      BuiltinFuncDecl(ASTContext& ctxt, BuiltinID id);

      /// Method for use by the ASTContext which loads the builtin
      /// with it \p id in the builtinFuncs_ map.
      void load(ASTContext& ctxt, BuiltinID id);

      Type type_;
      BuiltinID bID_;
  };

  /// VarDecl
  ///    A let or var variable declaration. 
  ///    This is simply a ValueDecl with an added "init" Expr*. It is
  ///    constant if declared using "let", non const if declared
  ///    using "var".
  class VarDecl final: public ValueDecl {
    public:
      enum class Keyword {
        Let, Var
        // No more room in this enum. If you want to add elements,
        // increase the size of the initAndKW_ pair.
      };

      /// Creates a new VarDecl
      /// \param ctxt the ASTContext in which the node will be allocated
      /// \param parent the parent DeclContext
      /// \param id the Identifier of the function
      /// \param idRange the range of the identifier of the function
      /// \param type the TypeLoc of the return type
      /// \param kw the Keyword (let or var) that was used to declare
      ///        this variable
      /// \param init the initializer expression, if present
      /// \param range the full SourceRange of the VarDecl. This should
      ///        not include the trailing semicolon.
      static VarDecl* create(ASTContext& ctxt, DeclContext* parent,
        Keyword kw, SourceRange kwRange, Identifier id, SourceRange idRange,
        TypeLoc type, Expr* init);

      Expr* getInitExpr() const;
      void setInitExpr(Expr* expr);
      bool hasInitExpr() const;

      TypeLoc getTypeLoc() const;

      /// \returns the ValueType for this VarDecl. This does not
      /// make a distinction between 'let' and 'var', it just
      /// returns the type as written down by the user.
      Type getValueType() const;

      SourceRange getSourceRange() const;

      /// \returns the SourceRange of the keyword used to declare this
      /// variable.
      SourceRange getKeywordRange() const;

      /// Returns true if this variable was declared using the "var" keyword
      /// (and thus, is mutable)
      bool isVar() const;
      /// Returns true if this variable was declared using the "let" keyword
      /// (and thus, is a constant)
      bool isLet() const;

      static bool classof(const Decl* decl) {
        return decl->getKind() == DeclKind::VarDecl;
      }

    private:
      VarDecl(DeclContext* parent, Keyword kw, SourceRange kwRange, 
        Identifier id, SourceRange idRange, TypeLoc type, Expr* init);

      TypeLoc typeLoc_;
      SourceRange kwRange_;
      llvm::PointerIntPair<Expr*, 1, Keyword> initAndKW_;
  };

  /// UnitDecl
  ///    Represents a parsed Source file.
  class UnitDecl final: public Decl, public DeclContext {
    public:
      static UnitDecl* create(ASTContext& ctxt, Identifier id, FileID file);

      /// \returns the Identifier of this UnitDecl
      Identifier getIdentifier() const;

      /// \returns the ASTContext this UnitDecl lives in.
      ASTContext& getASTContext() const;

      /// \returns the full SourceRange of this UnitDecl
      SourceRange getSourceRange() const;

      static bool classof(const Decl* decl) {
        return decl->getKind() == DeclKind::UnitDecl;
      }

      static bool classof(const DeclContext* dc) {
        return dc->getDeclContextKind() == DeclContextKind::UnitDecl;
      }

    private:
      UnitDecl(ASTContext& ctxt, Identifier id, FileID inFile);

      const FileID file_;
      Identifier identifier_;
      ASTContext& ctxt_;
  };
}

