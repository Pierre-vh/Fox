////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTDecl.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Declares the IASTDecl interface as well as derived nodes.
////------------------------------------------------------////

#pragma once
#include "ASTStmt.hpp"
#include "Moonshot/Fox/AST/IVisitor.hpp"
#include "Moonshot/Common/Types/Types.hpp"
#include <memory>
#include <vector>

namespace Moonshot
{
	// Forward declaration
	struct IASTExpr;
	struct IASTStmt;
	// Interface for Decl nodes.
	struct IASTDecl
	{
		public:
			IASTDecl() = default;
			virtual ~IASTDecl() = 0 {}
			virtual void accept(IVisitor& vis) = 0;
	};

	// "Adaptator" Interface for when a node is both a declaration and a statement (e.g. a variable declaration)
	struct IASTDeclStmt : public virtual IASTDecl, public virtual IASTStmt
	{
		~IASTDeclStmt() = 0 {}
	};

	// Store a arg's attribute : name, type, and if it's a reference.
	struct FoxFunctionArg : public FoxVariableAttr
	{
		public:
			FoxFunctionArg() = default;
			FoxFunctionArg(const std::string &nm, const std::size_t &ty, const bool isK, const bool& isref);

			bool isRef_;
			std::string dump() const;
			operator bool() const;
			bool operator==(const FoxFunctionArg& other) const;
			bool operator!=(const FoxFunctionArg& other) const;
		private:
			using FoxVariableAttr::wasInit_;
	};

	// a Function declaration node.
	struct ASTFunctionDecl : public IASTDecl
	{
		ASTFunctionDecl() = default;
		ASTFunctionDecl(const FoxType& returnType, const std::string& name, std::vector<FoxFunctionArg> args, std::unique_ptr<ASTCompoundStmt> funcbody);

		virtual void accept(IVisitor& vis) { vis.visit(*this); }

		FoxType returnType_;
		std::string name_;
		std::vector<FoxFunctionArg> args_;

		std::unique_ptr<ASTCompoundStmt> body_;
	};

	// A Variable declaration
	struct ASTVarDecl : public IASTDeclStmt
	{
		public:
			// Create a variable declaration statement by giving the constructor the variable's properties (name,is const and type) and, if there's one, an expression to initialize it.
			ASTVarDecl(const FoxVariableAttr &attr, std::unique_ptr<IASTExpr> iExpr);

			// Inherited via IASTStmt
			virtual void accept(IVisitor& vis) override;

			FoxVariableAttr vattr_;
			std::unique_ptr<IASTExpr> initExpr_ = nullptr;
	};
}

