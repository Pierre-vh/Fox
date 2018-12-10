//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : Decl.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// Declares the Decl hierarchy
//----------------------------------------------------------------------------//

#pragma once
#include "DeclContext.hpp"
#include "ASTAligns.hpp"
#include "Type.hpp"
#include "Identifier.hpp"

namespace fox {
  // Kinds of Decls
  enum class DeclKind : std::uint8_t {
    #define DECL(ID,PARENT) ID,
    #define DECL_RANGE(ID,FIRST,LAST) First_##ID = FIRST, Last_##ID = LAST,
    #include "DeclNodes.def"
  };

  // Forward Declarations
  class Expr;
  class Identifier;
  class ASTContext;
  class CompoundStmt;

  // Decl
  //    Common base class for every Declaration
  class alignas(align::DeclAlignement) Decl {
    public:
      DeclKind getKind() const;

      void setRange(SourceRange range);
      SourceRange getRange() const;

      bool isValid() const;

      void dump() const;

      bool isTopLevelDecl() const;
      void setIsTopLevelDecl(bool val);

      // Prohibit the use of builtin placement new & delete
      void *operator new(std::size_t) throw() = delete;
      void operator delete(void *) throw() = delete;
      void* operator new(std::size_t, void*) = delete;

      // Only allow allocation through the ASTContext
      void* operator new(std::size_t sz, ASTContext &ctxt, std::uint8_t align = alignof(Decl));

      // Companion operator delete to silence C4291 on MSVC
      void operator delete(void*, ASTContext&, std::uint8_t) {}

    protected:
      Decl(DeclKind kind, SourceRange range);

    private:
      void initBitfields();

      bool topLevel_ : 1;
      SourceRange range_;
      const DeclKind kind_;
  };

  // NamedDecl
  //    Common base class for every named Declaration
  class NamedDecl : public Decl {
    public:
      NamedDecl(DeclKind kind, Identifier id, SourceRange range);

      Identifier getIdentifier() const;
      void setIdentifier(Identifier id);
      bool hasIdentifier() const;

      bool isValid() const;

      static bool classof(const Decl* decl) {
        return (decl->getKind() >= DeclKind::First_NamedDecl) && (decl->getKind() <= DeclKind::Last_NamedDecl);
      }

    private:
      Identifier identifier_;
  };

  // ValueDecl
  //    Common base class for every value declaration 
  //    (declares a value of a certain type & name)
  class ValueDecl : public NamedDecl {
    public:
      ValueDecl(DeclKind kind, Identifier id, TypeLoc ty, 
        bool isConst, SourceRange range);

      Type getType() const;
      SourceRange getTypeRange() const;
      TypeLoc getTypeLoc() const;
      void setTypeLoc(TypeLoc ty);

      bool isConstant() const;
      void setIsConstant(bool k);

      bool isValid() const;

      static bool classof(const Decl* decl) {
        return (decl->getKind() >= DeclKind::First_ValueDecl) && (decl->getKind() <= DeclKind::Last_ValueDecl);
      }

    private:
      bool isConst_;
      TypeLoc type_;
  };

  // ParamDecl
  //    A declaration of a function parameter
  class ParamDecl : public ValueDecl {
    public:
      ParamDecl();
      ParamDecl(Identifier id, TypeLoc type, bool isConst, SourceRange range);

      bool isValid() const;

      static bool classof(const Decl* decl) {
        return decl->getKind() == DeclKind::ParamDecl;
      }
  };

  
  // FuncDecl
  //    A function declaration
  class FuncDecl : public NamedDecl, public DeclContext {
    private:
      using ParamVecTy = std::vector<ParamDecl*>;

      using ParamVecIter = ParamVecTy::iterator;
      using ParamVecConstIter = ParamVecTy::const_iterator;

    public:
      FuncDecl();
      FuncDecl(TypeLoc rtrTy, Identifier fnId, CompoundStmt* body,
        SourceRange range, SourceLoc headerEndLoc);
      
      void setLocs(SourceRange range, SourceLoc headerEndLoc);
      void setHeaderEndLoc(SourceLoc loc);

      SourceLoc getHeaderEndLoc() const;
      SourceRange getHeaderRange() const;

      // Note: Calls isValid on the args too.
      bool isValid() const;

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
      
      static bool classof(const DeclContext* dc) {
        return dc->getDeclContextKind() == DeclContextKind::FuncDecl;
      }

    private:
      SourceLoc headEndLoc_;
      TypeLoc returnType_;
      ParamVecTy params_;
      CompoundStmt* body_ = nullptr;

      // Bitfields (7 bits left)
      bool paramsAreValid_ : 1;
  };

  // VarDecl
  //    A variable declaration
  class VarDecl : public ValueDecl {
    public:
      VarDecl();
      VarDecl(Identifier id, TypeLoc type, bool isConst,
        Expr* init, SourceRange range);

      bool isValid() const;

      Expr* getInitExpr() const;
      void setInitExpr(Expr* expr);
      bool hasInitExpr() const;

      static bool classof(const Decl* decl) {
        return decl->getKind() == DeclKind::VarDecl;
      }

    private:
      Expr* init_ = nullptr;
  };

  // UnitDecl
  //    A Unit declaration (a unit is a source file)
  class UnitDecl : public NamedDecl, public DeclContext {
    private:
      using DeclVecTy = std::vector<Decl*>;
      using DeclVecIter = DeclVecTy::iterator;
      using DeclVecConstIter = DeclVecTy::const_iterator;

    public:
      UnitDecl(Identifier id, FileID inFile);

      void addDecl(Decl* decl);
      void setDecl(Decl* decl, std::size_t idx);
      Decl* getDecl(std::size_t idx) const;
      DeclVecTy& getDecls();
      std::size_t getNumDecls() const;

      bool isValid() const;

      FileID getFileID() const;
      void setFileID(const FileID& fid);

      static bool classof(const Decl* decl) {
        return decl->getKind() == DeclKind::UnitDecl;
      }

      static bool classof(const DeclContext* dc) {
        return dc->getDeclContextKind() == DeclContextKind::UnitDecl;
      }

    private:
      DeclVecTy decls_;
      FileID file_;

      // Bitfields (7 bits left)
      bool declsAreValid_ : 1;
  };
}

