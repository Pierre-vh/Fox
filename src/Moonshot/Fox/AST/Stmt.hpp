////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Stmt.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file declares the interface Stmt and it's derived classes.							
////------------------------------------------------------////

#pragma once

#include "Moonshot/Fox/Common/Memory.hpp"
#include "Moonshot/Fox/Common/SourceManager.hpp"

namespace Moonshot
{
	// The StmtKind enum
	enum class StmtKind : char
	{
		#define STMT(ID,PARENT) ID,
		#include "StmtNodes.def"
	};

	class Decl;
	class Expr;
	// Stmt interface
	class Stmt
	{
		public:
			Stmt(const StmtKind& skind);
			virtual ~Stmt() = 0 {}

			virtual bool isExpr() const;

			StmtKind getKind() const;
		private:
			StmtKind kind_;
	};

	// The return <expr> statement.
	class ReturnStmt : public Stmt
	{
		public:
			ReturnStmt(std::unique_ptr<Expr> rtr_expr = nullptr);

			bool hasExpr() const;
			Expr* getExpr();
			void setExpr(std::unique_ptr<Expr> e);
		private:
			std::unique_ptr<Expr> expr_;
	};

	// a if-then-else type condition.
	class ConditionStmt : public Stmt
	{
		public:
			ConditionStmt(std::unique_ptr<Expr> cond = nullptr, std::unique_ptr<Stmt> then = nullptr, std::unique_ptr<Stmt> elsestmt = nullptr);

			bool isValid() const;
			bool hasElse() const;

			Expr* getCond();
			Stmt* getThen();
			Stmt* getElse();

			void setCond(std::unique_ptr<Expr> expr);
			void setThen(std::unique_ptr<Stmt> then);
			void setElse(std::unique_ptr<Stmt> elsestmt);
		private:
			std::unique_ptr<Expr> cond_;
			std::unique_ptr<Stmt> then_;
			std::unique_ptr<Stmt> else_;
	};

	// A compound statement (statements between curly brackets)
	class CompoundStmt : public Stmt
	{
		private:
			using StmtVecTy = UniquePtrVector<Stmt>;
			using StmtVecIter = DereferenceIterator<StmtVecTy::iterator>;
			using StmtVecConstIter = DereferenceIterator < StmtVecTy::const_iterator>;
		public:
			CompoundStmt();

			Stmt* getStmt(const std::size_t& ind);
			Stmt* getBack();
			void addStmt(std::unique_ptr<Stmt> stmt);

			bool isEmpty() const;
			std::size_t size() const;

			StmtVecIter stmts_beg();
			StmtVecIter stmts_end();

			StmtVecConstIter stmts_beg() const;
			StmtVecConstIter stmts_end() const;

		private:
			StmtVecTy stmts_;
	};

	// A while loop while(expr) <stmt>
	class WhileStmt : public Stmt
	{
		public:
			WhileStmt(std::unique_ptr<Expr> cond = nullptr, std::unique_ptr<Stmt> body = nullptr);

			Expr* getCond();
			Stmt* getBody();

			void setCond(std::unique_ptr<Expr> cond);
			void setBody(std::unique_ptr<Stmt> body);
		private:
			std::unique_ptr<Expr> cond_;
			std::unique_ptr<Stmt> body_;
	};

	// Class used to mix Declarations & Statements without having decl inherit from stmt.
	// This is just a wrapper around a std::unique_ptr<Decl>
	class DeclStmt : public Stmt
	{
		public:
			DeclStmt(std::unique_ptr<Decl> decl = nullptr);

			bool hasDecl() const;
			Decl* getDecl();
			void setDecl(std::unique_ptr<Decl> decl);
		private:
			std::unique_ptr<Decl> decl_ = nullptr;			
	};
}

