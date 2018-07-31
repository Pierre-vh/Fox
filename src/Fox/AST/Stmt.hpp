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
		#define STMT_RANGE(ID,FIRST,LAST) First_##ID = FIRST, Last_##ID = LAST,
		#include "StmtNodes.def"
	};

	class Decl;
	class Expr;
	class ASTContext;

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

			// Prohibit the use of builtin placement new & delete
			void *operator new(std::size_t) throw() = delete;
			void operator delete(void *) throw() = delete;
			void* operator new(std::size_t, void*) = delete;

			// Only allow allocation through the ASTContext
			void* operator new(std::size_t sz, ASTContext &ctxt, std::uint8_t align = alignof(Stmt));

			// Companion operator delete to silence C4291 on MSVC
			void operator delete(void*, ASTContext&, std::uint8_t) {}

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

			static bool classof(const Stmt* stmt)
			{
				return (stmt->getKind() == StmtKind::NullStmt);
			}
	};

	// The "return" statement
	class ReturnStmt : public Stmt
	{
		public:
			ReturnStmt();
			ReturnStmt(Expr* rtr_expr, const SourceLoc& begLoc, const SourceLoc& endLoc);

			bool hasExpr() const;
			Expr* getExpr();
			void setExpr(Expr* e);

			static bool classof(const Stmt* stmt)
			{
				return (stmt->getKind() == StmtKind::ReturnStmt);
			}
		private:
			Expr* expr_ = nullptr;
	};

	// a if-then-else condition.
	class ConditionStmt : public Stmt
	{
		public:
			ConditionStmt();
			ConditionStmt(Expr* cond, Stmt* then, Stmt* elsestmt,
				const SourceLoc& begLoc, const SourceLoc& ifHeaderEndLoc, const SourceLoc& endLoc);

			bool isValid() const;
			bool hasElse() const;

			Expr* getCond();
			Stmt* getThen();
			Stmt* getElse();

			const Expr* getCond() const;
			const Stmt* getThen() const;
			const Stmt* getElse() const;

			void setCond(Expr* expr);
			void setThen(Stmt* stmt);
			void setElse(Stmt* stmt);

			void setIfHeaderEndLoc(const SourceLoc& sloc);
			SourceRange getIfHeaderRange() const;
			SourceLoc getIfHeaderEndLoc() const;

			static bool classof(const Stmt* stmt)
			{
				return (stmt->getKind() == StmtKind::ConditionStmt);
			}
		private:
			SourceLoc ifHeadEndLoc_;

			Expr* cond_ = nullptr;
			Stmt* then_ = nullptr;
			Stmt* else_ = nullptr;
	};

	// A compound statement (statements between curly brackets)
	class CompoundStmt : public Stmt
	{
		private:
			using StmtVecTy = std::vector<Stmt*>;
			using StmtVecIter = StmtVecTy::iterator;
			using StmtVecConstIter = StmtVecTy::const_iterator;
		public:
			CompoundStmt();
			CompoundStmt(const SourceLoc& begLoc, const SourceLoc& endLoc);

			Stmt* getStmt(std::size_t ind);
			const Stmt* getStmt(std::size_t ind) const;

			Stmt* getBack();
			const Stmt* getBack() const;

			void addStmt(Stmt* stmt);

			bool isEmpty() const;
			std::size_t size() const;

			StmtVecIter stmts_beg();
			StmtVecIter stmts_end();

			StmtVecConstIter stmts_beg() const;
			StmtVecConstIter stmts_end() const;

			// begLoc and endLoc = the locs of the curly brackets.
			void setSourceLocs(const SourceLoc& begLoc, const SourceLoc& endLoc);

			static bool classof(const Stmt* stmt)
			{
				return (stmt->getKind() == StmtKind::CompoundStmt);
			}
		private:
			StmtVecTy stmts_;
	};

	// A while loop while(expr) <stmt>
	class WhileStmt : public Stmt
	{
		public:
			WhileStmt();
			WhileStmt(Expr* cond, Stmt* body, const SourceLoc& begLoc, const SourceLoc& headerEndLoc, const SourceLoc& endLoc);

			Expr* getCond();
			Stmt* getBody();

			const Expr* getCond() const;
			const Stmt* getBody() const;

			void setCond(Expr* cond);
			void setBody(Stmt* body);

			SourceLoc getHeaderEndLoc() const;
			SourceRange getHeaderRange() const;

			static bool classof(const Stmt* stmt)
			{
				return (stmt->getKind() == StmtKind::WhileStmt);
			}
		private:
			SourceLoc headerEndLoc_;
			Expr* cond_ = nullptr;
			Stmt* body_ = nullptr;
	};

	// Class used to mix Declarations & Statements without having decl inherit from stmt.
	// This is just a wrapper around a Decl*
	class DeclStmt : public Stmt
	{
		public:
			DeclStmt(Decl* decl);

			bool hasDecl() const;

			Decl* getDecl();
			const Decl* getDecl() const;

			void setDecl(Decl* decl);

			static bool classof(const Stmt* stmt)
			{
				return (stmt->getKind() == StmtKind::DeclStmt);
			}
		private:
			Decl* decl_ = nullptr;			
	};
}

