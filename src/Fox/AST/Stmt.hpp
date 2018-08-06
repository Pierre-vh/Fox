////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Stmt.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file declares the interface Stmt and it's derived classes.							
////------------------------------------------------------////

#pragma once

#include "Fox/Common/Source.hpp"
#include "ASTNode.hpp"
#include <vector>

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

			Expr* getExpr();
			void setExpr(Expr* e);
			bool hasExpr() const;

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
			ConditionStmt(Expr* cond, ASTNode then, ASTNode elsenode,
				const SourceLoc& begLoc, const SourceLoc& ifHeaderEndLoc, const SourceLoc& endLoc);

			bool isValid() const;

			void setCond(Expr* expr);
			Expr* getCond() const;

			void setThen(ASTNode node);
			ASTNode getThen() const;

			void setElse(ASTNode node);
			ASTNode getElse() const;
			bool hasElse() const;

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
			ASTNode then_, else_;
	};

	// A compound statement (statements between curly brackets)
	class CompoundStmt : public Stmt
	{
		private:
			using NodeVecTy = std::vector<ASTNode>;

		public:
			CompoundStmt();
			CompoundStmt(const SourceLoc& begLoc, const SourceLoc& endLoc);

			void addNode(ASTNode stmt);
			ASTNode getNode(std::size_t ind) const;
			bool isEmpty() const;
			std::size_t size() const;

			NodeVecTy::iterator nodes_begin();
			NodeVecTy::iterator nodes_end();

			NodeVecTy::const_iterator nodes_begin() const;
			NodeVecTy::const_iterator nodes_end() const;

			// begLoc and endLoc = the locs of the curly brackets.
			void setSourceLocs(const SourceLoc& begLoc, const SourceLoc& endLoc);

			static bool classof(const Stmt* stmt)
			{
				return (stmt->getKind() == StmtKind::CompoundStmt);
			}

		private:
			NodeVecTy nodes_;
	};

	// A while loop while(expr) <stmt>
	class WhileStmt : public Stmt
	{
		public:
			WhileStmt();
			WhileStmt(Expr* cond, ASTNode body, const SourceLoc& begLoc, const SourceLoc& headerEndLoc, const SourceLoc& endLoc);

			void setCond(Expr* cond);
			Expr* getCond() const;

			void setBody(ASTNode body);
			ASTNode getBody() const;

			SourceLoc getHeaderEndLoc() const;
			SourceRange getHeaderRange() const;

			static bool classof(const Stmt* stmt)
			{
				return (stmt->getKind() == StmtKind::WhileStmt);
			}

		private:
			SourceLoc headerEndLoc_;
			Expr* cond_ = nullptr;
			ASTNode body_;
	};
}

