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
	enum class DeclKind : char
	{
		#define DECL(ID,PARENT) ID,
		#include "DeclNodes.def"
	};

	class SourceRange;
	class SourceLoc;
	class Expr;
	class IdentifierInfo;
	class IVisitor;

	class Decl
	{
		public:
			Decl(const DeclKind& dkind,const SourceLoc& begLoc, const SourceLoc& endLoc);
			virtual ~Decl() = 0 {}
			
			DeclKind getKind() const;

			SourceLoc getBegLoc() const;
			SourceLoc getEndLoc() const;
			SourceRange getRange() const;

			bool isLocationAvailable() const;
		protected:
			friend class Parser;

			bool isBegLocSet() const;
			bool isEndLocSet() const;

			void setBegLoc(const SourceLoc& loc);
			void setEndLoc(const SourceLoc& loc);
		private:
			SourceLoc begLoc_, endLoc_;
			DeclKind kind_;
	};

	// Base class for Declarations that have names, e.g. : var/arg/func decl,..
	class NamedDecl : public Decl
	{
		public:
			NamedDecl(const DeclKind& dkind,IdentifierInfo* name,const SourceLoc& begLoc, const SourceLoc& endLoc);

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
			ArgDecl(IdentifierInfo* id, const QualType& argType,const SourceLoc& begLoc, const SourceLoc& endLoc);

			QualType getType() const;
			void setType(const QualType& qt);

			bool isComplete() const;
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
			FunctionDecl();

			// Note : DeclEndLoc shall include the body of the function
			// todo: Make the parser include it once CompoundStmt has everything it needs.
			FunctionDecl(Type* returnType, IdentifierInfo* fnId, std::unique_ptr<CompoundStmt> funcbody, const SourceLoc& begLoc,const SourceLoc& declEndLoc);

			bool isComplete() const;

			void setReturnType(Type* ty);
			Type* getReturnType();
			const Type* getReturnType() const;

			void setBody(std::unique_ptr<CompoundStmt> arg);
			CompoundStmt* getBody();	
			const CompoundStmt* getBody() const;

			ArgDecl* getArg(const std::size_t & ind);
			const ArgDecl* getArg(const std::size_t & ind) const;
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
			VarDecl(IdentifierInfo * varId,const QualType& ty, std::unique_ptr<Expr> iExpr, const SourceLoc& begLoc, const SourceLoc& endLoc);

			bool isComplete() const;

			// Get a reference to the varType
			QualType getType() const;
			void setType(const QualType &ty);

			Expr* getInitExpr();
			const Expr* getInitExpr() const;

			void setInitExpr(std::unique_ptr<Expr> expr);
			bool hasInitExpr() const;
		private:
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
			const Decl* getDecl(const std::size_t &idx) const;

			std::size_t getDeclCount() const;

			bool isComplete() const;

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

