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

	// IASTStmt interface
	struct IASTStmt 
	{
		public:
			IASTStmt() = default;
			virtual ~IASTStmt() = 0 {}
			virtual void accept(IVisitor& vis) = 0;
	};

	// A null statement, that doesn't do anything. (It's a placeholder)
	// It's going to be ignored most of the time, isn't that sad?
	struct ASTNullStmt : public IASTStmt	
	{
		ASTNullStmt() = default;
		virtual void accept(IVisitor& vis) override;
	};

	// The return <expr> statement.
	struct ASTReturnStmt : public IASTStmt	
	{
		ASTReturnStmt() = default;
		ASTReturnStmt(std::unique_ptr<IASTExpr> rtr_expr);

		virtual void accept(IVisitor& vis) override;

		bool hasExpr() const;
		std::unique_ptr<IASTExpr> expr_;
	};

	// a if-then-else type condition.
	struct ASTCondStmt : public IASTStmt
	{
		public:
			ASTCondStmt() = default;

			virtual void accept(IVisitor & vis) override;

			std::unique_ptr<IASTExpr> cond_;
			std::unique_ptr<IASTStmt> then_;
			std::unique_ptr<IASTStmt> else_;
	};

	// A compound statement (statements between curly brackets)
	struct ASTCompoundStmt : public IASTStmt
	{
		public:
			ASTCompoundStmt() = default;

			virtual void accept(IVisitor & vis) override;

			std::vector<std::unique_ptr<IASTStmt>> statements_;
	};

	// A while loop while(expr) <stmt>
	struct ASTWhileStmt : public IASTStmt
	{
		public:
			ASTWhileStmt() = default;

			virtual void accept(IVisitor & vis) override;

			std::unique_ptr<IASTExpr> expr_;
			std::unique_ptr<IASTStmt> body_;
	};
}

