////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : IASTStmt.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file declares the interface IASTStmt and it's derived classes.							
////------------------------------------------------------////

#pragma once

#include "Moonshot/Fox/AST/IVisitor.hpp"
#include <memory>
#include <vector>

namespace Moonshot
{
	struct IASTExpr;

	struct IASTStmt 
	{
		public:
			IASTStmt() = default;
			virtual ~IASTStmt() = 0 {}
			virtual void accept(IVisitor& vis) = 0;
	};

	struct ASTNullStmt : public IASTStmt	// A null statement, that doesn't do anything. It's going to be ignored most of the time, isn't that sad?
	{
		ASTNullStmt() = default;
		virtual void accept(IVisitor& vis) override;
	};

	struct ASTReturnStmt : public IASTStmt	
	{
		ASTReturnStmt() = default;
		ASTReturnStmt(std::unique_ptr<IASTExpr> rtr_expr);

		virtual void accept(IVisitor& vis) override;

		bool hasExpr() const;
		std::unique_ptr<IASTExpr> expr_;
	};

	struct ASTCondStmt : public IASTStmt
	{
		public:
			ASTCondStmt() = default;

			virtual void accept(IVisitor & vis) override;

			std::unique_ptr<IASTExpr> cond_;
			std::unique_ptr<IASTStmt> then_;
			std::unique_ptr<IASTStmt> else_;
	};

	struct ASTCompoundStmt : public IASTStmt
	{
		public:
			ASTCompoundStmt() = default;

			virtual void accept(IVisitor & vis) override;

			std::vector<std::unique_ptr<IASTStmt>> statements_;
	};

	struct ASTWhileStmt : public IASTStmt
	{
		public:
			ASTWhileStmt() = default;

			virtual void accept(IVisitor & vis) override;

			std::unique_ptr<IASTExpr> expr_;
			std::unique_ptr<IASTStmt> body_;
	};
}

