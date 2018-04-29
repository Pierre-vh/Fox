////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Expr.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Declares the Expr interface as well as derived nodes. 
////------------------------------------------------------////

#pragma once
#include "Moonshot/Fox/Common/Typedefs.hpp"
#include "Moonshot/Fox/AST/Stmt.hpp"
#include "Moonshot/Fox/AST/Type.hpp"
#include "Moonshot/Fox/AST/Operators.hpp"
#include "Moonshot/Fox/Common/Memory.hpp"

namespace Moonshot	
{
	class IVisitor;
	class IdentifierInfo;
	// base expression 
	class Expr : public Stmt
	{
		public:
			Expr(const StmtKind& kind);
			inline virtual ~Expr() = 0 {}
			virtual void accept(IVisitor& vis) = 0;

			virtual bool isExpr() const override;
	};

	// A Null expression that's just a placeholder.
	// Mostly used by the parser to recover from situations, it doesn't necessarily
	// represent any language construction.
	class NullExpr : public Expr
	{
		public:
			NullExpr();
			virtual void accept(IVisitor &vis) override;
	};

	// Binary Expressions
	class BinaryExpr : public Expr
	{
		public:
			BinaryExpr() = default;
			BinaryExpr(const binaryOperator &opt,std::unique_ptr<Expr> lhs = nullptr,std::unique_ptr<Expr> rhs = nullptr);

			virtual void accept(IVisitor& vis) override;
			std::unique_ptr<Expr> getSimple();	// If there is no right node and the optype is "pass", this will move and return the left node 

			Expr* getLHS();
			Expr* getRHS();

			void setLHS(std::unique_ptr<Expr> nlhs);
			void setRHS(std::unique_ptr<Expr> nrhs);

			binaryOperator getOp() const;
			void setOp(const binaryOperator& op);

			// Returns true if node has both a left_ and right_ child and op != default
			bool isComplete() const; 
		private:
			std::unique_ptr<Expr> left_, right_;
			binaryOperator op_ = binaryOperator::DEFAULT;
	};

	// Unary Expressions
	class UnaryExpr : public Expr
	{
		public: 
			UnaryExpr() = default;
			UnaryExpr(const unaryOperator& opt,std::unique_ptr<Expr> node = nullptr);
			virtual void accept(IVisitor& vis) override;

			Expr* getChild();
			void setChild(std::unique_ptr<Expr> nchild);

			unaryOperator getOp() const;
			void setOp(const unaryOperator& nop);

		private:
			std::unique_ptr<Expr> child_;
			unaryOperator op_ = unaryOperator::DEFAULT;
	};

	// Explicit Cast Expressions
	class CastExpr : public Expr
	{
		public:
			CastExpr(const Type* castGoal,std::unique_ptr<Expr> child = nullptr);
			
			virtual void accept(IVisitor& vis) override;

			void setCastGoal(const Type* goal);
			const Type* getCastGoal() const;

			Expr* getChild();
			void setChild(std::unique_ptr<Expr> nc);
		private:
			const Type* goal_ = nullptr;
			std::unique_ptr<Expr> child_;
	};

	// Literals
	class CharLiteralExpr : public Expr
	{
		public:
			CharLiteralExpr(const CharType &val);

			void accept(IVisitor &vis) override;

			CharType getVal() const;
			void setVal(const CharType& val);
		private:
			CharType val_ = ' ';
	};

	class IntegerLiteralExpr : public Expr
	{
		public:
			IntegerLiteralExpr() = default;
			IntegerLiteralExpr(const IntType &val);

			void accept(IVisitor &vis) override;

			IntType getVal() const;
			void setVal(const IntType& val);
		private:
			IntType val_ = 0;
	};

	class FloatLiteralExpr : public Expr
	{
		public:
			FloatLiteralExpr() = default;
			FloatLiteralExpr(const FloatType &val);

			void accept(IVisitor &vis) override;

			FloatType getVal() const;
			void setVal(const FloatType& val);
		private:
			FloatType val_ = 0.0f;
	};

	class StringLiteralExpr : public Expr
	{
		public:
			StringLiteralExpr() = default;
			StringLiteralExpr(const std::string &val);

			void accept(IVisitor &vis) override;

			std::string getVal() const;
			void setVal(const std::string& val);
		private:
			std::string val_ = "";
	};

	class BoolLiteralExpr : public Expr
	{
		public:
			BoolLiteralExpr() = default;
			BoolLiteralExpr(const bool &val);

			void accept(IVisitor &vis) override;

			bool getVal() const;
			void setVal(const bool& val);
		private:
			bool val_ = false;
	};

	class ExprList;
	// Array literals
	class ArrayLiteralExpr : public Expr
	{
		public:
			ArrayLiteralExpr(std::unique_ptr<ExprList> exprs = nullptr);

			ExprList* getExprList();
			void setExprList(std::unique_ptr<ExprList> elist);
			bool hasExprList() const; 

			bool isEmpty() const;

			virtual void accept(IVisitor &vis);
		private:
			std::unique_ptr<ExprList> exprs_;
	};

	// Represents a reference to a declaration (namespace,variable,function) -> it's an identifier!
	class DeclRefExpr : public Expr
	{
		public:
			DeclRefExpr(IdentifierInfo * declid);

			void accept(IVisitor& vis) override;
			
			IdentifierInfo * getDeclIdentifier();
			void setDeclIdentifier(IdentifierInfo * id);
		private:
			IdentifierInfo * declId_;
	};

	// Arrays accesses : foo[0], etc.
	class ArrayAccessExpr : public Expr
	{
		public:
			ArrayAccessExpr(std::unique_ptr<Expr> expr, std::unique_ptr<Expr> idxexpr);
			void accept(IVisitor& vis) override;

			void setBase(std::unique_ptr<Expr> expr);
			void setAccessIndexExpr(std::unique_ptr<Expr> expr);

			Expr* getBase() ;
			Expr* getAccessIndexExpr();
		private:
			// 2 Expr, the expression supposed to produce an array, and the expression contained within the square brackets that should produce the index.
			std::unique_ptr<Expr> base_;
			std::unique_ptr<Expr> accessIdxExpr_;
	};

	// Node Representing an Expression List.
		// Note: This is not a "normal" node (not visitable nor inherited from expr), 
		// it's more of a wrapper around a std::vector<std::unique_ptr<Expr>>, so we can pass a list of 
		// expressions around easily.
	class ExprList
	{
		private:
			using ExprListTy = UniquePtrVector<Expr>;

			using ExprListIter = DereferenceIterator<ExprListTy::iterator>;
			using ExprListConstIter = DereferenceIterator<ExprListTy::const_iterator>;
		public:
			ExprList() = default;

			void addExpr(std::unique_ptr<Expr> expr);
			Expr* getExpr(const std::size_t& ind);

			bool isEmpty() const;
			std::size_t size() const;

			ExprListIter begin();
			ExprListIter end();

			ExprListConstIter begin() const;
			ExprListConstIter end()const;
		private:
			ExprListTy exprs_;
	};

	// Function calls
	class FunctionCallExpr : public Expr
	{
		public:
			FunctionCallExpr() = default;
			FunctionCallExpr(std::unique_ptr<DeclRefExpr> base, std::unique_ptr<ExprList> elist = nullptr);

			DeclRefExpr * getCallee() ;
			void setCallee(std::unique_ptr<DeclRefExpr> base);

			ExprList* getExprList();
			void setExprList(std::unique_ptr<ExprList> elist);

			void accept(IVisitor& vis) override;
		private:
			// the Function's name
			std::unique_ptr<DeclRefExpr> callee_;
			// it's args
			std::unique_ptr<ExprList> args_;
	};
}

