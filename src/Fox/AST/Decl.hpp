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
		#include "DeclNodes.def"
	};

	class SourceRange;
	class SourceLoc;
	class Expr;
	class IdentifierInfo;
	class IVisitor;

	// Note about SourceLocs in decls:
	// The getRange,getBegLoc and getEndLoc shall always return the complete range of the decl, including any potential children.
	class Decl
	{
		public:
			virtual ~Decl() = 0 {};

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
	protected:
			Decl(DeclKind kind, const SourceLoc& begLoc, const SourceLoc& endLoc);
		private:
			SourceLoc begLoc_, endLoc_;
			DeclKind kind_;
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
		private:
			IdentifierInfo* identifier_;
	};

	// A Function Argument declaration
	class ArgDecl : public NamedDecl
	{
		public:
			ArgDecl();
			ArgDecl(IdentifierInfo* id, const QualType& type,const SourceLoc& begLoc, const SourceRange& tyRange, const SourceLoc& endLoc);

			SourceRange getTypeRange() const;

			QualType getType() const;
			void setType(const QualType& qt);

			bool isValid() const;
	private:
			SourceRange tyRange_;
			QualType type_;
	};

	// Represents a function declaration.
	// This class is a DeclContext because it has its own scope
	// separated from the Unit's scope. If this wasn't a DeclContext,
	// every variable inside a Function would be visible in the global scope,
	// making name resolution much harder.
	class FunctionDecl : public NamedDecl, public DeclContext
	{
		private:
			using ArgVecTy = UniquePtrVector<ArgDecl>;

			using ArgVecIter = DereferenceIterator<ArgVecTy::iterator>;
			using ArgVecConstIter = DereferenceIterator<ArgVecTy::const_iterator>;
		public:
			FunctionDecl();
			FunctionDecl(Type* returnType, IdentifierInfo* fnId, std::unique_ptr<CompoundStmt> body, const SourceLoc& begLoc,const SourceLoc& headerEndLoc,const SourceLoc& endLoc);
			
			void setSourceLocs(const SourceLoc& beg, const SourceLoc& declEnd, const SourceLoc& end);
			void setHeaderEndLoc(const SourceLoc& loc);

			SourceLoc getHeaderEndLoc() const;
			SourceRange getHeaderRange() const;

			// Note: Calls isValid on the args too.
			bool isValid() const;

			void setReturnType(Type* ty);
			Type* getReturnType();
			const Type* getReturnType() const;

			void setBody(std::unique_ptr<CompoundStmt> arg);
			CompoundStmt* getBody();	
			const CompoundStmt* getBody() const;

			void addArg(std::unique_ptr<ArgDecl> arg);
			ArgDecl* getArg(std::size_t ind);
			const ArgDecl* getArg(std::size_t ind) const;
			std::size_t argsSize() const;

			ArgVecIter args_begin();
			ArgVecConstIter args_begin() const;

			ArgVecIter args_end();
			ArgVecConstIter args_end() const;
		private:
			SourceLoc headEndLoc_;
			Type* returnType_ = nullptr;
			ArgVecTy args_;
			std::unique_ptr<CompoundStmt> body_;

			// Bitfields
			bool argsAreValid_ : 1;
	};

	// A Variable declaration
	class VarDecl : public NamedDecl
	{
		public:
			VarDecl();
			VarDecl(IdentifierInfo * id, const QualType& type, std::unique_ptr<Expr> initializer, const SourceLoc& begLoc, const SourceRange& tyRange, const SourceLoc& endLoc);

			bool isValid() const;

			SourceRange getTypeRange() const;

			QualType getType() const;
			void setType(const QualType &ty);

			Expr* getInitExpr();
			const Expr* getInitExpr() const;

			void setInitExpr(std::unique_ptr<Expr> expr);
			bool hasInitExpr() const;
		private:
			SourceRange typeRange_;
			QualType type_;
			std::unique_ptr<Expr> initializer_;
	};

	// A Unit declaration. A Unit = a source file.
	// Unit names?
	class UnitDecl : public NamedDecl, public DeclContext
	{
		private:
			using DelVecTy = UniquePtrVector<Decl>;
			using DeclVecIter = DereferenceIterator<DelVecTy::iterator>;
			using DeclVecConstIter = DereferenceIterator<DelVecTy::const_iterator>;
		public:
			UnitDecl(IdentifierInfo *id, FileID inFile);

			void addDecl(std::unique_ptr<Decl> decl);

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
		private:
			DelVecTy decls_;
			FileID file_;

			// Bitfields
			bool declsAreValid_ : 1;
	};
}

