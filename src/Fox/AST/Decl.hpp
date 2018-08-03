////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Decl.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Declares the Decl interface as well as derived nodes.
////------------------------------------------------------////

#pragma once
#include "Type.hpp"
#include "Stmt.hpp"
#include "DeclContext.hpp"

namespace fox
{
	enum class DeclKind : std::uint8_t
	{
		#define DECL(ID,PARENT) ID,
		#define DECL_RANGE(ID,FIRST,LAST) First_##ID = FIRST, Last_##ID = LAST,
		#include "DeclNodes.def"
	};

	class Expr;
	class IdentifierInfo;
	class ASTContext;

	// Note about SourceLocs in decls:
	// The getRange,getBegLoc and getEndLoc shall always return the complete range of the decl, including any potential children.
	class Decl
	{
		public:
			DeclKind getKind() const;

			SourceLoc getBegLoc() const;
			SourceLoc getEndLoc() const;
			SourceRange getRange() const;

			bool isBegLocSet() const;
			bool isEndLocSet() const;

			void setBegLoc(const SourceLoc& loc);
			void setEndLoc(const SourceLoc& loc);

			bool hasLocInfo() const;

			bool isValid() const;

			// Prohibit the use of builtin placement new & delete
			void *operator new(std::size_t) throw() = delete;
			void operator delete(void *) throw() = delete;
			void* operator new(std::size_t, void*) = delete;

			// Only allow allocation through the ASTContext
			void* operator new(std::size_t sz, ASTContext &ctxt, std::uint8_t align = alignof(Decl));

			// Companion operator delete to silence C4291 on MSVC
			void operator delete(void*, ASTContext&, std::uint8_t) {}

		protected:
			Decl(DeclKind kind, const SourceLoc& begLoc, const SourceLoc& endLoc);

		private:
			SourceLoc begLoc_, endLoc_;
			const DeclKind kind_;
	};

	// Base class for Declarations that have names, e.g. : var/arg/func decl,..
	class NamedDecl : public Decl
	{
		public:
			NamedDecl(DeclKind kind,IdentifierInfo* id,const SourceLoc& begLoc, const SourceLoc& endLoc);

			IdentifierInfo * getIdentifier() const;
			void setIdentifier(IdentifierInfo* nname);

			bool hasIdentifier() const;
			bool isValid() const;

			static bool classof(const Decl* decl)
			{
				return (decl->getKind() >= DeclKind::First_NamedDecl) && (decl->getKind() <= DeclKind::Last_NamedDecl);
			}

		private:
			IdentifierInfo* identifier_;
	};

	// A Function Parameter declaration
	class ParamDecl : public NamedDecl
	{
		public:
			ParamDecl();
			ParamDecl(IdentifierInfo* id, const QualType& type,const SourceLoc& begLoc, const SourceRange& tyRange, const SourceLoc& endLoc);

			SourceRange getTypeRange() const;

			QualType getType() const;
			void setType(const QualType& qt);

			bool isValid() const;

			static bool classof(const Decl* decl)
			{
				return decl->getKind() == DeclKind::ParamDecl;
			}

		private:
			SourceRange tyRange_;
			QualType type_;
	};

	// Represents a function declaration.
	// This class is a DeclContext because it has its own scope
	// separated from the Unit's scope. If this wasn't a DeclContext,
	// every variable inside a Function would be visible in the global scope,
	// making name resolution much harder.
	class FuncDecl : public NamedDecl, public DeclContext
	{
		private:
			using ParamVecTy = std::vector<ParamDecl*>;

			using ParamVecIter = ParamVecTy::iterator;
			using ParamVecConstIter = ParamVecTy::const_iterator;

		public:
			FuncDecl();
			FuncDecl(Type* returnType, IdentifierInfo* fnId, CompoundStmt* body, const SourceLoc& begLoc,const SourceLoc& headerEndLoc,const SourceLoc& endLoc);
			
			void setSourceLocs(const SourceLoc& beg, const SourceLoc& declEnd, const SourceLoc& end);
			void setHeaderEndLoc(const SourceLoc& loc);

			SourceLoc getHeaderEndLoc() const;
			SourceRange getHeaderRange() const;

			// Note: Calls isValid on the args too.
			bool isValid() const;

			void setReturnType(Type* ty);
			Type* getReturnType();
			const Type* getReturnType() const;

			void setBody(CompoundStmt* body);
			CompoundStmt* getBody();	
			const CompoundStmt* getBody() const;

			void addParam(ParamDecl* arg);
			ParamDecl* getParamDecl(std::size_t ind);
			const ParamDecl* getParamDecl(std::size_t ind) const;
			std::size_t getNumParams() const;

			ParamVecIter params_begin();
			ParamVecConstIter params_begin() const;

			ParamVecIter params_end();
			ParamVecConstIter params_end() const;

			static bool classof(const Decl* decl)
			{
				return decl->getKind() == DeclKind::FuncDecl;
			}

		private:
			SourceLoc headEndLoc_;
			Type* returnType_ = nullptr;
			ParamVecTy params_;
			CompoundStmt* body_ = nullptr;

			// Bitfields
			bool paramsAreValid_ : 1;
	};

	// A Variable declaration
	class VarDecl : public NamedDecl
	{
		public:
			VarDecl();
			VarDecl(IdentifierInfo * id, const QualType& type, Expr* init, const SourceLoc& begLoc, const SourceRange& tyRange, const SourceLoc& endLoc);

			bool isValid() const;

			SourceRange getTypeRange() const;

			QualType getType() const;
			void setType(const QualType &ty);

			Expr* getInitExpr();
			const Expr* getInitExpr() const;

			void setInitExpr(Expr* expr);
			bool hasInitExpr() const;

			static bool classof(const Decl* decl)
			{
				return decl->getKind() == DeclKind::VarDecl;
			}

		private:
			SourceRange typeRange_;
			QualType type_;
			Expr* init_ = nullptr;
	};

	// A Unit declaration. A Unit = a source file.
	// Unit names?
	class UnitDecl : public NamedDecl, public DeclContext
	{
		private:
			using DelVecTy = std::vector<Decl*>;
			using DeclVecIter = DelVecTy::iterator;
			using DeclVecConstIter = DelVecTy::const_iterator;

		public:
			UnitDecl(IdentifierInfo *id, FileID inFile);

			void addDecl(Decl* decl);

			Decl* getDecl(std::size_t idx);
			const Decl* getDecl(std::size_t idx) const;

			std::size_t getDeclCount() const;

			// Note: Checks the validity of the decls 
			// inside this too.
			bool isValid() const;

			DeclVecIter decls_beg();
			DeclVecIter decls_end();

			DeclVecConstIter decls_beg() const;
			DeclVecConstIter decls_end() const;

			FileID getFileID() const;
			void setFileID(const FileID& fid);

			static bool classof(const Decl* decl)
			{
				return decl->getKind() == DeclKind::UnitDecl;
			}

		private:
			DelVecTy decls_;
			FileID file_;

			// Bitfields (7 bits left)
			bool declsAreValid_ : 1;
	};
}

