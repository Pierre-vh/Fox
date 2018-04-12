////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTStmt.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file declares the interface ASTStmt and it's derived classes.							
////------------------------------------------------------////

#pragma once

#include <memory>
#include <vector>
#include <functional>

namespace Moonshot
{
	class ASTExpr;
	class IVisitor;

	// ASTStmt interface
	class ASTStmt
	{
		public:
			ASTStmt() = default;
			virtual ~ASTStmt() = 0 {}
			virtual void accept(IVisitor& vis) = 0;
	};

	// A null statement, that doesn't do anything. (It's a placeholder)
	// It's going to be ignored most of the time, isn't that sad?
	class ASTNullStmt : public ASTStmt
	{
		public:
			ASTNullStmt() = default;
			virtual void accept(IVisitor& vis) override;
	};

	// The return <expr> statement.
	class ASTReturnStmt : public ASTStmt
	{
		public:
			ASTReturnStmt(std::unique_ptr<ASTExpr> rtr_expr = nullptr);

			virtual void accept(IVisitor& vis) override;

			bool hasExpr() const;
			ASTExpr* getExpr();
			void setExpr(std::unique_ptr<ASTExpr> e);
		private:
			std::unique_ptr<ASTExpr> expr_;
	};

	// a if-then-else type condition.
	class ASTCondStmt : public ASTStmt
	{
		public:
			ASTCondStmt(std::unique_ptr<ASTExpr> cond = nullptr, std::unique_ptr<ASTStmt> then = nullptr, std::unique_ptr<ASTStmt> elsestmt = nullptr);

			virtual void accept(IVisitor & vis) override;

			bool isValid() const;
			bool hasElse() const;

			ASTExpr* getCond();
			ASTStmt* getThen();
			ASTStmt* getElse();

			void setCond(std::unique_ptr<ASTExpr> expr);
			void setThen(std::unique_ptr<ASTStmt> then);
			void setElse(std::unique_ptr<ASTStmt> elsestmt);
		private:
			std::unique_ptr<ASTExpr> cond_;
			std::unique_ptr<ASTStmt> then_;
			std::unique_ptr<ASTStmt> else_;
	};

	// A compound statement (statements between curly brackets)
	class ASTCompoundStmt : public ASTStmt
	{
		private:
			using stmtvec = std::vector<std::unique_ptr<ASTStmt>>;
		public:
			ASTCompoundStmt() = default;

			virtual void accept(IVisitor & vis) override;

			ASTStmt* getStmt(const std::size_t& ind);
			ASTStmt* getBack(); // returns the .back() of the stmtvec
			void addStmt(std::unique_ptr<ASTStmt> stmt);

			bool isEmpty() const;
			std::size_t size() const;

			stmtvec::iterator stmtList_beg();
			stmtvec::iterator stmtList_end();

			void iterateStmts(std::function<void(ASTStmt*)> fn);
		private:
			stmtvec stmts_;
	};

	// A while loop while(expr) <stmt>
	class ASTWhileStmt : public ASTStmt
	{
		public:
			ASTWhileStmt(std::unique_ptr<ASTExpr> cond = nullptr, std::unique_ptr<ASTStmt> body = nullptr);

			virtual void accept(IVisitor & vis) override;

			ASTExpr* getCond();
			ASTStmt* getBody();

			void setCond(std::unique_ptr<ASTExpr> cond);
			void setBody(std::unique_ptr<ASTStmt> body);
		private:
			std::unique_ptr<ASTExpr> cond_;
			std::unique_ptr<ASTStmt> body_;
	};
}

