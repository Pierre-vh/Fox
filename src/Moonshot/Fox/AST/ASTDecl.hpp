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

#include <memory>
#include <vector>

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

			IdentifierInfo * getDeclName() const;
			void setDeclName(IdentifierInfo* nname);
		protected:
			IdentifierInfo * declName_;
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
	class ASTFunctionDecl : public ASTNamedDecl
	{
		private:
			using ArgVecTy = std::vector<std::unique_ptr<ASTArgDecl>>;

			using argIter = ArgVecTy::iterator;
			using argIter_const = ArgVecTy::const_iterator;
		public:
			ASTFunctionDecl() = default;
			ASTFunctionDecl(const Type* returnType, IdentifierInfo* fnId, std::unique_ptr<ASTCompoundStmt> funcbody);

			virtual void accept(IVisitor& vis) override;
			virtual bool isValid() override;

			void setReturnType(const Type* ty);
			const Type* getReturnType() const;

			void setBody(std::unique_ptr<ASTCompoundStmt> arg);
			ASTCompoundStmt* getBody();		

			const ASTArgDecl* getArg(const std::size_t & ind) const;
			void addArg(std::unique_ptr<ASTArgDecl> arg);
			std::size_t argsSize() const;

			argIter args_begin();
			argIter_const args_begin() const;

			argIter args_end();
			argIter_const args_end() const;
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
}

