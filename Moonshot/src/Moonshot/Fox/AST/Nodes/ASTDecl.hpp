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
	struct IASTExpr;

	struct IASTDecl : public IASTStmt
	{
		public:
			IASTDecl() = default;
			virtual ~IASTDecl() = 0 {}
			virtual void accept(IVisitor& vis) = 0;
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
		private:
			using FoxVariableAttr::wasInit_;
	};

	inline bool operator==(const FoxFunctionArg& lhs, const FoxFunctionArg& rhs)
	{
		return (lhs.name_ == rhs.name_) && (lhs.type_ == rhs.type_);
	}

	inline bool operator!=(const FoxFunctionArg& lhs, const FoxFunctionArg& rhs)
	{
		return !(lhs == rhs);
	}

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

	struct ASTVarDecl : public IASTDecl
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

