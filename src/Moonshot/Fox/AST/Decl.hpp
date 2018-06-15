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
#include "DeclRecorder.hpp"
#include "Moonshot/Fox/Common/Memory.hpp"

namespace Moonshot
{
	// The DeclKind enum
	enum class DeclKind : char
	{
		#define DECL(ID,PARENT) ID,
		#include "DeclNodes.def"
	};

	// Forward declarations
	class Expr;
	class IdentifierInfo;
	class IVisitor;
	// Interface for Decl nodes.
	class Decl
	{
		public:
			Decl(const DeclKind& dkind);
			virtual ~Decl() = 0 {}
			
			DeclKind getKind() const;
		private:
			DeclKind kind_;
	};

	// Base class for Declarations that have names, e.g. : var/arg/func decl,..
	class NamedDecl : public Decl
	{
		public:
			NamedDecl(const DeclKind& dkind,IdentifierInfo* name);

			IdentifierInfo * getIdentifier() const;
			void setIdentifier(IdentifierInfo* nname);
			bool hasIdentifier() const;
		private:
			IdentifierInfo * identifier_;
	};

	// A Function Argument declaration
	class ArgDecl : public NamedDecl
	{
		public:
			ArgDecl(IdentifierInfo* id, const QualType& argType);

			QualType getType() const;
			void setType(const QualType& qt);

			bool isValid();
		private:
			QualType ty_;
	};

	// a Function declaration node.
	class FunctionDecl : public NamedDecl, public DeclRecorder
	{
		private:
			using ArgVecTy = UniquePtrVector<ArgDecl>;

			using ArgVecIter = DereferenceIterator<ArgVecTy::iterator>;
			using ArgVecConstIter = DereferenceIterator<ArgVecTy::const_iterator>;
		public:
			FunctionDecl(Type* returnType = nullptr, IdentifierInfo* fnId = nullptr, std::unique_ptr<CompoundStmt> funcbody = nullptr);

			bool isValid();

			void setReturnType(Type* ty);
			Type* getReturnType();

			void setBody(std::unique_ptr<CompoundStmt> arg);
			CompoundStmt* getBody();		

			ArgDecl* getArg(const std::size_t & ind);
			void addArg(std::unique_ptr<ArgDecl> arg);
			std::size_t argsSize() const;

			ArgVecIter args_begin();
			ArgVecConstIter args_begin() const;

			ArgVecIter args_end();
			ArgVecConstIter args_end() const;
		private:
			Type* returnType_ = nullptr;
			ArgVecTy args_;
			std::unique_ptr<CompoundStmt> body_;
	};

	// A Variable declaration
	class VarDecl : public NamedDecl
	{
		public:
			VarDecl(IdentifierInfo * varId = nullptr,const QualType& ty = QualType(), std::unique_ptr<Expr> iExpr = nullptr);

			bool isValid();

			// Get a reference to the varType
			QualType getType() const;
			void setType(const QualType &ty);

			Expr* getInitExpr();
			void setInitExpr(std::unique_ptr<Expr> expr);

			bool hasInitExpr() const;

			void setAllLocs(const SourceLoc& idLoc, const SourceLoc& typeLoc, const SourceLoc& semiLoc);
			
			void setIDLoc(const SourceLoc& sloc);
			SourceLoc getIDLoc() const;

			void setTypeLoc(const SourceLoc& sloc);
			SourceLoc getTypeLoc() const;

			void setSemiLoc(const SourceLoc& sloc);
			SourceLoc getSemiLoc() const;
		private:
			SourceLoc idLoc_, typeLoc_, semiLoc_;
			QualType varTy_;
			std::unique_ptr<Expr> initExpr_ = nullptr;
	};

	// A Unit declaration. A Unit = a source file.
	// Unit names?
	class UnitDecl : public NamedDecl, public DeclRecorder
	{
		private:
			using DelVecTy = UniquePtrVector<Decl>;
			using DeclVecIter = DereferenceIterator<DelVecTy::iterator>;
			using DeclVecConstIter = DereferenceIterator<DelVecTy::const_iterator>;
		public:
			UnitDecl(IdentifierInfo *id, const FileID& fid);

			void addDecl(std::unique_ptr<Decl> decl);
			Decl* getDecl(const std::size_t &idx);
			std::size_t getDeclCount() const;

			bool isValid();

			DeclVecIter decls_beg();
			DeclVecIter decls_end();

			DeclVecConstIter decls_beg() const;
			DeclVecConstIter decls_end() const;

			FileID getFileID() const;
			void setFileID(const FileID& fid);
		private:
			// The decls contained within this unit.
			FileID fid_;
			DelVecTy decls_;
	};
}

