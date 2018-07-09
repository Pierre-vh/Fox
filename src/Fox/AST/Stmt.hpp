////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Stmt.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file declares the interface Stmt and it's derived classes.							
////------------------------------------------------------////

#pragma once

#include "Fox/Common/Iterators.hpp"
#include "Fox/Common/Source.hpp"

namespace fox
{
	// The StmtKind enum
	enum class StmtKind : std::uint8_t
	{
		#define STMT(ID,PARENT) ID,
		#include "StmtNodes.def"
	};

	class Decl;
	class Expr;

	/*
	
		A Note about SourceLoc info in statements (and expressions)

		Every node should provide SourceLoc/Ranges for the whole node, including the childrens +
		any relevant range if applicable.

		e.g. a Condition should give us a 
			Complete range (from the "if" to the "}")
			A Range for the if condition ("if" to ")")
	*/

	// Base Stmt Class
	class Stmt
	{
		public:
			virtual ~Stmt() = 0 {};

			bool isExpr() const;

			StmtKind getKind() const;

			SourceRange getRange() const;

			SourceLoc getBegLoc() const;
			SourceLoc getEndLoc() const;

			void setBegLoc(const SourceLoc& loc);
			void setEndLoc(const SourceLoc& loc);

			bool isBegLocSet() const;
			bool isEndLocSet() const;

			bool hasLocInfo() const;
		protected:
			Stmt(StmtKind skind, const SourceLoc& begLoc, const SourceLoc& endLoc);
		private:
			SourceLoc beg_, end_;
			const StmtKind kind_;
	};

	// The ';' statement.
	class NullStmt : public Stmt
	{
		public:
			NullStmt();
			NullStmt(const SourceLoc& semiLoc);

			void setSemiLoc(const SourceLoc& semiLoc);
			SourceLoc getSemiLoc() const;
	};

	// The "return" statement
	class ReturnStmt : public Stmt
	{
		public:
			ReturnStmt();
			ReturnStmt(std::unique_ptr<Expr> rtr_expr, const SourceLoc& begLoc, const SourceLoc& endLoc);

			bool hasExpr() const;
			Expr* getExpr();
			void setExpr(std::unique_ptr<Expr> e);
		private:
			std::unique_ptr<Expr> expr_;
	};

	// a if-then-else condition.
	class ConditionStmt : public Stmt
	{
		public:
			ConditionStmt();
			ConditionStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> then, std::unique_ptr<Stmt> elsestmt,
				const SourceLoc& begLoc, const SourceLoc& ifHeaderEndLoc, const SourceLoc& endLoc);

			bool isValid() const;
			bool hasElse() const;

			Expr* getCond();
			Stmt* getThen();
			Stmt* getElse();

			const Expr* getCond() const;
			const Stmt* getThen() const;
			const Stmt* getElse() const;

			void setCond(std::unique_ptr<Expr> expr);
			void setThen(std::unique_ptr<Stmt> then);
			void setElse(std::unique_ptr<Stmt> elsestmt);

			void setIfHeaderEndLoc(const SourceLoc& sloc);
			SourceRange getIfHeaderRange() const;
			SourceLoc getIfHeaderEndLoc() const;
		private:
			SourceLoc ifHeadEndLoc_;

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
			CompoundStmt(const SourceLoc& begLoc, const SourceLoc& endLoc);

			Stmt* getStmt(std::size_t ind);
			const Stmt* getStmt(std::size_t ind) const;

			Stmt* getBack();
			const Stmt* getBack() const;

			void addStmt(std::unique_ptr<Stmt> stmt);

			bool isEmpty() const;
			std::size_t size() const;

			StmtVecIter stmts_beg();
			StmtVecIter stmts_end();

			StmtVecConstIter stmts_beg() const;
			StmtVecConstIter stmts_end() const;

			// begLoc and endLoc = the locs of the curly brackets.
			void setSourceLocs(const SourceLoc& begLoc, const SourceLoc& endLoc);
		private:
			StmtVecTy stmts_;
	};

	// A while loop while(expr) <stmt>
	class WhileStmt : public Stmt
	{
		public:
			WhileStmt();
			WhileStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> body, const SourceLoc& begLoc, const SourceLoc& headerEndLoc, const SourceLoc& endLoc);

			Expr* getCond();
			Stmt* getBody();

			const Expr* getCond() const;
			const Stmt* getBody() const;

			void setCond(std::unique_ptr<Expr> cond);
			void setBody(std::unique_ptr<Stmt> body);

			SourceLoc getHeaderEndLoc() const;
			SourceRange getHeaderRange() const;
		private:
			SourceLoc headerEndLoc_;
			std::unique_ptr<Expr> cond_;
			std::unique_ptr<Stmt> body_;
	};

	// Class used to mix Declarations & Statements without having decl inherit from stmt.
	// This is just a wrapper around a std::unique_ptr<Decl>
	class DeclStmt : public Stmt
	{
		public:
			DeclStmt(std::unique_ptr<Decl> decl);

			bool hasDecl() const;

			Decl* getDecl();
			const Decl* getDecl() const;

			void setDecl(std::unique_ptr<Decl> decl);
		private:
			std::unique_ptr<Decl> decl_ = nullptr;			
	};
}

