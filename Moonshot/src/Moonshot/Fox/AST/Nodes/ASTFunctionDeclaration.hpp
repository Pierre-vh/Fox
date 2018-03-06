////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTFunctionDeclaration.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// The AST Node for Function Declarations										
////------------------------------------------------------////

#pragma once
#include "IASTDeclaration.hpp"
#include "ASTCompoundStmt.hpp"
#include "Moonshot/Common/Types/Types.hpp"

namespace Moonshot
{
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

	struct ASTFunctionDeclaration : public IASTDeclaration	
	{
		ASTFunctionDeclaration() = default;
		ASTFunctionDeclaration(const FoxType& returnType,const std::string& name,std::vector<FoxFunctionArg> args, std::unique_ptr<ASTCompoundStmt> funcbody);

		virtual void accept(IVisitor& vis) { vis.visit(*this); }

		FoxType returnType_;
		std::string name_;
		std::vector<FoxFunctionArg> args_;

		std::unique_ptr<ASTCompoundStmt> body_;
	};
}