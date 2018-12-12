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
#include "DeclContext.hpp"
#include "ASTAligns.hpp"
#include "Type.hpp"
#include "Identifier.hpp"

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
    #include "DeclNodes.def"
  };

  // Decl
  //    Common base class for every Declaration
  //    Note that every Decl will take a DeclContext* argument. That DeclContext
  //    should be their parent DeclContext.
  class alignas(DeclAlignement) Decl {
    public:
      using Parent = llvm::PointerUnion<DeclContext*, FuncDecl*>;

      // Get the kind of Decl this is.
      DeclKind getKind() const;

      // For normal Decls, return the DeclContext in which
      // this Decl is referenced. Returns nullptr for
      // normal decls.
      DeclContext* getDeclContext() const;

      // Returns true if this is a local declaration
      bool isLocal() const;

      // For local decls, returns the FuncDecl in which
      // this declaration lives. For non local decls, returns nullptr.
      FuncDecl* getFuncDecl() const;

      // Returns the "closest" DeclContext.
      //  -> If this Decl is also a DeclContext, returns 
      //      dyn_cast<DeclContext>(this)
      //  -> else, if this Decl is a local Decl, returns 
      //      getFuncDecl()->getDeclContext()
      //  -> Else, returns getDeclContext()
      // Should never return nullptr in a well-formed AST.
      DeclContext* getClosestDeclContext() const;

      void setRange(SourceRange range);
      SourceRange getRange() const;

      // Get the FileID of the file where this Decl is located
      FileID getFile() const;

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

      // Companion operator delete to silence C4291 on MSVC
      void operator delete(void*, ASTContext&, std::uint8_t) {}

      Decl(DeclKind kind, Parent parent, SourceRange range);

    private:
      Parent parent_;
      SourceRange range_;
      const DeclKind kind_;
  };

  // NamedDecl
  //    Common base class for every named declaration
  //    (a declaration with an identifier)
  class NamedDecl : public Decl {
    public:
      Identifier getIdentifier() const;
      void setIdentifier(Identifier id);
      bool hasIdentifier() const;

      static bool classof(const Decl* decl) {
        return (decl->getKind() >= DeclKind::First_NamedDecl) 
          && (decl->getKind() <= DeclKind::Last_NamedDecl);
      }

    protected:
      NamedDecl(DeclKind kind, DeclContext* parent, 
        Identifier id, SourceRange range);

    private:
      Identifier identifier_;
  };

  // ValueDecl
  //    Common base class for every value declaration 
  //    (declares a value of a certain type & name)
  class ValueDecl : public NamedDecl {
    public:
      Type getType() const;
      SourceRange getTypeRange() const;
      TypeLoc getTypeLoc() const;
      void setTypeLoc(TypeLoc ty);

      bool isConstant() const;
      void setIsConstant(bool k);

      static bool classof(const Decl* decl) {
        return (decl->getKind() >= DeclKind::First_ValueDecl) 
          && (decl->getKind() <= DeclKind::Last_ValueDecl);
      }
    protected:
      ValueDecl(DeclKind kind, DeclContext* parent, Identifier id,
        TypeLoc ty, bool isConst, SourceRange range);

    private:
      bool isConst_;
      TypeLoc type_;
  };

  // ParamDecl
  //    A declaration of a function parameter. This is simply a ValueDecl.
  class ParamDecl final : public ValueDecl {
    public:
      static ParamDecl* create(ASTContext& ctxt, DeclContext* parent,
        Identifier id, TypeLoc type, bool isConst, SourceRange range);

      static bool classof(const Decl* decl) {
        return decl->getKind() == DeclKind::ParamDecl;
      }

    private:
      ParamDecl(DeclContext* parent, Identifier id, TypeLoc type, bool isConst,
        SourceRange range);
  };

  
  // FuncDecl
  //    A function declaration
  class FuncDecl final: public NamedDecl {
    private:
      using ParamVecTy = std::vector<ParamDecl*>;

      using ParamVecIter = ParamVecTy::iterator;
      using ParamVecConstIter = ParamVecTy::const_iterator;

    public:
      static FuncDecl* create(ASTContext& ctxt, DeclContext* parent,
        Identifier id, TypeLoc type, SourceRange range, SourceLoc headerEnd);

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

      void addParam(ParamDecl* param);
      void setParam(ParamDecl* param, std::size_t idx);
      void setParams(ParamVecTy&& params);

      ParamDecl* getParam(std::size_t ind) const;
      ParamVecTy& getParams();
      std::size_t getNumParams() const;

      static bool classof(const Decl* decl) {
        return decl->getKind() == DeclKind::FuncDecl;
      }
      
    private:
      FuncDecl(DeclContext* parent, Identifier fnId, TypeLoc rtrTy,
        SourceRange range, SourceLoc headerEndLoc);

      SourceLoc headEndLoc_;
      TypeLoc returnType_;
      ParamVecTy params_;
      CompoundStmt* body_ = nullptr;
  };

  // VarDecl
  //    A variable declaration. This is simply a ValueDecl with an added
  //    "init" Expr*
  class VarDecl final: public ValueDecl {
    public:
      static VarDecl* create(ASTContext& ctxt, DeclContext* parent,
        Identifier id, TypeLoc type, bool isConst, Expr* init,
        SourceRange range);

      Expr* getInitExpr() const;
      void setInitExpr(Expr* expr);
      bool hasInitExpr() const;

      static bool classof(const Decl* decl) {
        return decl->getKind() == DeclKind::VarDecl;
      }

    private:
      VarDecl(DeclContext* parent, Identifier id, TypeLoc type, bool isConst,
        Expr* init, SourceRange range);

      Expr* init_ = nullptr;
  };

  // UnitDecl
  //    Represents a parsed Source file. This is both a NamedDecl and a 
  //    DeclContext.
  class UnitDecl final: public NamedDecl, public DeclContext {
    public:
      static UnitDecl* create(ASTContext& ctxt, DeclContext* parent,
        Identifier id, FileID file);

      FileID getFileID() const;
      void setFileID(const FileID& fid);

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

      ASTContext& ctxt_;
      FileID file_;
  };
}

