////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTDecl.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Declares the ASTDecl interface as well as derived nodes.
////------------------------------------------------------////

#pragma once
#include "ASTStmt.hpp"
#include "Types.hpp"
#include "DeclRecorder.hpp"
#include "Moonshot/Fox/Basic/Memory.hpp"

namespace Moonshot
{
	// Forward declarations
	class ASTExpr;
	class IdentifierInfo;
	class IVisitor;
	// Interface for Decl nodes.
	class ASTDecl
	{
		public:
			ASTDecl() = default;
			virtual ~ASTDecl() = 0 {}
			virtual void accept(IVisitor& vis) = 0;

			// This function should return true if the declaration node is valid (usable)
			virtual bool isValid() = 0; 
	};

	// Base class for Declarations that have names, e.g. : var/arg/func decl,..
	class ASTNamedDecl : public virtual ASTDecl
	{
		public:
			ASTNamedDecl() = default;
			ASTNamedDecl(IdentifierInfo* name);

			IdentifierInfo * getIdentifier() const;
			void setIdentifier(IdentifierInfo* nname);
			bool hasIdentifier() const;
		private:
			IdentifierInfo * Ident_;
	};

	// A Function Argument declaration
	class ASTArgDecl : public ASTNamedDecl
	{
		public:
			ASTArgDecl() = default;
			ASTArgDecl(IdentifierInfo* id, const QualType& argType);

			QualType getType() const;
			void setType(const QualType& qt);

			virtual void accept(IVisitor &vis);
			virtual bool isValid() override;
		private:
			QualType ty_;
	};

	// a Function declaration node.
	class ASTFunctionDecl : public ASTNamedDecl, public DeclRecorder
	{
		private:
			using ArgVecTy = UniquePtrVector<ASTArgDecl>;

			using ArgVecIter = DereferenceIterator<ArgVecTy::iterator>;
			using ArgVecConstIter = DereferenceIterator<ArgVecTy::const_iterator>;
		public:
			ASTFunctionDecl() = default;
			ASTFunctionDecl(const Type* returnType, IdentifierInfo* fnId, std::unique_ptr<ASTCompoundStmt> funcbody);

			virtual void accept(IVisitor& vis) override;
			virtual bool isValid() override;

			void setReturnType(const Type* ty);
			const Type* getReturnType() const;

			void setBody(std::unique_ptr<ASTCompoundStmt> arg);
			ASTCompoundStmt* getBody();		

			ASTArgDecl* getArg(const std::size_t & ind);
			void addArg(std::unique_ptr<ASTArgDecl> arg);
			std::size_t argsSize() const;

			ArgVecIter args_begin();
			ArgVecConstIter args_begin() const;

			ArgVecIter args_end();
			ArgVecConstIter args_end() const;
		private:
			const Type* returnType_ = nullptr;
			ArgVecTy args_;
			std::unique_ptr<ASTCompoundStmt> body_;
	};

	// A Variable declaration
	class ASTVarDecl : public ASTNamedDecl, public ASTStmt
	{
		public:
			ASTVarDecl() = default;
			ASTVarDecl(IdentifierInfo * varId,const QualType& ty, std::unique_ptr<ASTExpr> iExpr = nullptr);

			virtual void accept(IVisitor& vis) override;
			virtual bool isValid() override;

			// Get a reference to the varType
			QualType getType() const;
			void setType(const QualType &ty);

			ASTExpr* getInitExpr();
			void setInitExpr(std::unique_ptr<ASTExpr> expr);

			bool hasInitExpr() const;

		private:
			QualType varTy_;
			std::unique_ptr<ASTExpr> initExpr_ = nullptr;
	};

	// A Unit declaration. A Unit = a source file.
		// Unit names?
	class ASTUnitDecl : public ASTNamedDecl, public DeclRecorder
	{
		private:
			using DelVecTy = UniquePtrVector<ASTDecl>;
			using DeclVecIter = DereferenceIterator<DelVecTy::iterator>;
			using DeclVecConstIter = DereferenceIterator<DelVecTy::const_iterator>;
		public:
			ASTUnitDecl(IdentifierInfo *id);

			void addDecl(std::unique_ptr<ASTDecl> decl);
			ASTDecl* getDecl(const std::size_t &idx);
			std::size_t getDeclCount() const;

			virtual bool isValid() override;
			virtual void accept(IVisitor &vis);

			DeclVecIter decls_beg();
			DeclVecIter decls_end();

			DeclVecConstIter decls_beg() const;
			DeclVecConstIter decls_end() const;

		private:
			// The decls contained within this unit.
			DelVecTy decls_;
	};
}

