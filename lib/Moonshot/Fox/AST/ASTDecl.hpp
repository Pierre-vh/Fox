////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTDecl.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Declares the IASTDecl interface as well as derived nodes.
////------------------------------------------------------////

#pragma once

#include "Moonshot/Common/Types/Types.hpp"

#include "ASTStmt.hpp"

#include <memory>
#include <vector>
#include <functional>

namespace Moonshot
{
	// Forward declaration
	class IASTExpr;

	class IVisitor;

	// Interface for Decl nodes.
	class IASTDecl
	{
		public:
			IASTDecl() = default;
			virtual ~IASTDecl() = 0 {}
			virtual void accept(IVisitor& vis) = 0;
	};

	// "Adaptator" Interface for when a node is both a declaration and a statement (e.g. a variable declaration)
	class IASTDeclStmt : public virtual IASTDecl, public virtual IASTStmt
	{
		public:
			~IASTDeclStmt() = 0 {}
	};

	// Store a arg's attribute : name, type, and if it's a reference.
	class FoxFunctionArg : public FoxVariableAttr
	{
		public:
			FoxFunctionArg() = default;
			FoxFunctionArg(const std::string &nm, const std::size_t &ty, const bool &isK, const bool& isref);

			bool isRef() const;
			void setIsRef(const bool& nref);

			bool isConst() const;
			void setConst(const bool& k);

			std::string dump() const;
			operator bool() const;
			bool operator==(const FoxFunctionArg& other) const;
			bool operator!=(const FoxFunctionArg& other) const;
		private:
			bool isRef_;
			using FoxVariableAttr::wasInit_;
	};

	// a Function declaration node.
	class ASTFunctionDecl : public IASTDecl
	{
		public:
			ASTFunctionDecl() = default;
			ASTFunctionDecl(const FoxType& returnType, const std::string& name, std::vector<FoxFunctionArg> args, std::unique_ptr<ASTCompoundStmt> funcbody);

			virtual void accept(IVisitor& vis) override;

			FoxType getReturnType() const;
			std::string getName() const;
			FoxFunctionArg getArg(const std::size_t &ind) const;
			ASTCompoundStmt* getBody();

			void setReturnType(const FoxType& ft);
			void setName(const std::string& str);
			void setArgs(const std::vector<FoxFunctionArg>& vec);
			void addArg(const FoxFunctionArg& arg);
			void setBody(std::unique_ptr<ASTCompoundStmt> arg);
		
			void iterateArgs(std::function<void(FoxFunctionArg)> fn);
		private:
			FoxType returnType_;
			std::string name_;
			std::vector<FoxFunctionArg> args_;

			std::unique_ptr<ASTCompoundStmt> body_;
	};

	// A Variable declaration
	class ASTVarDecl : public IASTDeclStmt
	{
		public:
			// Create a variable declaration statement by giving the constructor the variable's properties (name,is const and type) and, if there's one, an expression to initialize it.
			ASTVarDecl(const FoxVariableAttr &attr, std::unique_ptr<IASTExpr> iExpr = nullptr);

			// Inherited via IASTStmt
			virtual void accept(IVisitor& vis) override;

			FoxVariableAttr getVarAttr() const;
			IASTExpr* getInitExpr();

			bool hasInitExpr() const;

			void setVarAttr(const FoxVariableAttr& vattr);
			void setInitExpr(std::unique_ptr <IASTExpr> expr);
		private:
			FoxVariableAttr vattr_;
			std::unique_ptr<IASTExpr> initExpr_ = nullptr;
	};
}

