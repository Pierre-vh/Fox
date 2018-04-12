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
	// Forward declaration
	class ASTExpr;

	class IVisitor;

	// Interface for Decl nodes.
	class ASTDecl
	{
		public:
			ASTDecl() = default;
			virtual ~ASTDecl() = 0 {}
			virtual void accept(IVisitor& vis) = 0;
	};

	// "Adaptator" Interface for when a node is both a declaration and a statement (e.g. a variable declaration)
	class IASTDeclStmt : public virtual ASTDecl, public virtual ASTStmt
	{
		public:
			~IASTDeclStmt() = 0 {}
	};

	// Store a arg's attribute : it's name and QualType
	class FunctionArg
	{
		public:
			FunctionArg() = default;
			FunctionArg(const std::string& argName, const QualType& argType);

			std::string getArgName() const;
			void setArgName(const std::string& name);

			QualType getQualType() const;
			void setQualType(const QualType& qt);

		private:
			QualType ty_;
			std::string name_;
	};

	// a Function declaration node.
	class ASTFunctionDecl : public ASTDecl
	{
		private:
			using argIter = std::vector<FunctionArg>::iterator;
			using argIter_const = std::vector<FunctionArg>::const_iterator;
		public:
			ASTFunctionDecl() = default;
			ASTFunctionDecl(Type* returnType, const std::string& name, std::vector<FunctionArg> args, std::unique_ptr<ASTCompoundStmt> funcbody);

			virtual void accept(IVisitor& vis) override;

			void setReturnType(Type *ty);
			Type* getReturnType();

			std::string getName() const;
			void setName(const std::string& str);

			void setArgs(const std::vector<FunctionArg>& vec);
			void addArg(const FunctionArg& arg);
			FunctionArg getArg(const std::size_t &ind) const;

			void setBody(std::unique_ptr<ASTCompoundStmt> arg);
			ASTCompoundStmt* getBody();		

			argIter args_begin();
			argIter_const args_begin() const;

			argIter args_end();
			argIter_const args_end() const;
		private:
			Type * returnType_ = nullptr;
			std::string name_;
			std::vector<FunctionArg> args_;

			std::unique_ptr<ASTCompoundStmt> body_;
	};

	// A Variable declaration
	class ASTVarDecl : public IASTDeclStmt
	{
		public:
			ASTVarDecl(const std::string& varname,const QualType& ty, std::unique_ptr<ASTExpr> iExpr = nullptr);

			virtual void accept(IVisitor& vis) override;

			QualType getVarTy();
			ASTExpr* getInitExpr();

			std::string getVarName() const;
			void setVarName(const std::string& name);

			bool hasInitExpr() const;

			void setVarType(const QualType &ty);
			void setInitExpr(std::unique_ptr <ASTExpr> expr);
		private:
			QualType varTy_;
			std::string varName_;
			std::unique_ptr<ASTExpr> initExpr_ = nullptr;
	};
}

