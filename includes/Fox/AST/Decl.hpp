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
  //    Note that every Decl will take a DeclContext* argument. That DeclContext
  //    should be their parent DeclContext.
  class alignas(DeclAlignement) Decl {
    public:
      DeclKind getKind() const;

      void setRange(SourceRange range);
      SourceRange getRange() const;
      // Returns the FileID of the file where this decl is located.
      FileID getFile() const;

      void dump() const;

      // Prohibit the use of builtin placement new & delete
      void *operator new(std::size_t) throw() = delete;
      void operator delete(void *) throw() = delete;
      void* operator new(std::size_t, void*) = delete;

      // Only allow allocation through the ASTContext
      void* operator new(std::size_t sz, ASTContext &ctxt, std::uint8_t align = alignof(Decl));

      // Companion operator delete to silence C4291 on MSVC
      void operator delete(void*, ASTContext&, std::uint8_t) {}

    protected:
      Decl(DeclKind kind, DeclContext* parent, SourceRange range);

    private:
      DeclContext* parent_ = nullptr;
      SourceRange range_;
      const DeclKind kind_;
  };

  // NamedDecl
  //    Common base class for every named Declaration
  class NamedDecl : public Decl {
    public:
      NamedDecl(DeclKind kind, DeclContext* parent, Identifier id, SourceRange range);

      Identifier getIdentifier() const;
      void setIdentifier(Identifier id);
      bool hasIdentifier() const;

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
      ValueDecl(DeclKind kind, DeclContext* parent, Identifier id,
        TypeLoc ty, bool isConst, SourceRange range);

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

    private:
      bool isConst_;
      TypeLoc type_;
  };

  // ParamDecl
  //    A declaration of a function parameter
  class ParamDecl : public ValueDecl {
    public:
      ParamDecl();
      ParamDecl(DeclContext* parent, Identifier id, TypeLoc type, bool isConst,
        SourceRange range);

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
      FuncDecl(DeclContext* parent, TypeLoc rtrTy, Identifier fnId,
        CompoundStmt* body,SourceRange range, SourceLoc headerEndLoc);
      
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
      
      static bool classof(const DeclContext* dc) {
        return dc->getDeclContextKind() == DeclContextKind::FuncDecl;
      }

    private:
      SourceLoc headEndLoc_;
      TypeLoc returnType_;
      ParamVecTy params_;
      CompoundStmt* body_ = nullptr;
  };

  // VarDecl
  //    A variable declaration
  class VarDecl : public ValueDecl {
    public:
      VarDecl();
      VarDecl(DeclContext* parent, Identifier id, TypeLoc type, bool isConst,
        Expr* init, SourceRange range);

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
  //    Represents a Source file.
  class UnitDecl : public NamedDecl, public DeclContext {
    public:
      UnitDecl(ASTContext& ctxt, DeclContext* parent, Identifier id, 
        FileID inFile);

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
      ASTContext& ctxt_;
      FileID file_;
  };
}

