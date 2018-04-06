////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTStmt.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file declares the interface IASTStmt and it's derived classes.							
////------------------------------------------------------////

#pragma once

#include <memory>
#include <vector>
#include <functional>

namespace Moonshot
{
	class IASTExpr;
	class IVisitor;

	// IASTStmt interface
	class IASTStmt
	{
		public:
			IASTStmt() = default;
			virtual ~IASTStmt() = 0 {}
			virtual void accept(IVisitor& vis) = 0;
	};

	// A null statement, that doesn't do anything. (It's a placeholder)
	// It's going to be ignored most of the time, isn't that sad?
	class ASTNullStmt : public IASTStmt
	{
		public:
			ASTNullStmt() = default;
			virtual void accept(IVisitor& vis) override;
	};

	// The return <expr> statement.
	class ASTReturnStmt : public IASTStmt
	{
		public:
			ASTReturnStmt(std::unique_ptr<IASTExpr> rtr_expr = nullptr);

			virtual void accept(IVisitor& vis) override;

			bool hasExpr() const;
			IASTExpr* getExpr();
			void setExpr(std::unique_ptr<IASTExpr> e);
		private:
			std::unique_ptr<IASTExpr> expr_;
	};

	// a if-then-else type condition.
	class ASTCondStmt : public IASTStmt
	{
		public:
			ASTCondStmt(std::unique_ptr<IASTExpr> cond = nullptr, std::unique_ptr<IASTStmt> then = nullptr, std::unique_ptr<IASTStmt> elsestmt = nullptr);

			virtual void accept(IVisitor & vis) override;

			bool isValid() const;
			bool hasElse() const;

			IASTExpr* getCond();
			IASTStmt* getThen();
			IASTStmt* getElse();

			void setCond(std::unique_ptr<IASTExpr> expr);
			void setThen(std::unique_ptr<IASTStmt> then);
			void setElse(std::unique_ptr<IASTStmt> elsestmt);
		private:
			std::unique_ptr<IASTExpr> cond_;
			std::unique_ptr<IASTStmt> then_;
			std::unique_ptr<IASTStmt> else_;
	};

	// A compound statement (statements between curly brackets)
	class ASTCompoundStmt : public IASTStmt
	{
		private:
			using stmtvec = std::vector<std::unique_ptr<IASTStmt>>;
		public:
			ASTCompoundStmt() = default;

			virtual void accept(IVisitor & vis) override;

			IASTStmt* getStmt(const std::size_t& ind);
			IASTStmt* getBack(); // returns the .back() of the stmtvec
			void addStmt(std::unique_ptr<IASTStmt> stmt);

			bool isEmpty() const;
			std::size_t size() const;

			stmtvec::iterator stmtList_beg();
			stmtvec::iterator stmtList_end();

			void iterateStmts(std::function<void(IASTStmt*)> fn);
		private:
			stmtvec stmts_;
	};

	// A while loop while(expr) <stmt>
	class ASTWhileStmt : public IASTStmt
	{
		public:
			ASTWhileStmt(std::unique_ptr<IASTExpr> cond = nullptr, std::unique_ptr<IASTStmt> body = nullptr);

			virtual void accept(IVisitor & vis) override;

			IASTExpr* getCond();
			IASTStmt* getBody();

			void setCond(std::unique_ptr<IASTExpr> cond);
			void setBody(std::unique_ptr<IASTStmt> body);
		private:
			std::unique_ptr<IASTExpr> cond_;
			std::unique_ptr<IASTStmt> body_;
	};
}

