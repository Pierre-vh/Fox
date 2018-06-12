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
#include "Moonshot/Fox/Common/Memory.hpp"

namespace Moonshot	
{
	// Operators Enums & Dictionaries
	enum class binaryOperator
	{
		DEFAULT,
		CONCAT,	// +
		// Basic math ops
		ADD,	// +
		MINUS,	// -
		MUL,	// *
		DIV,	// /
		MOD,	// %
		EXP,	// **
		// Logical and and or
		LOGIC_AND,	// &&
		LOGIC_OR,	// ||
		// Comparison
		LESS_OR_EQUAL,		// <=
		GREATER_OR_EQUAL,	// >=
		LESS_THAN,			// <
		GREATER_THAN,		// >
		EQUAL,				// ==
		NOTEQUAL,			// !=

		// Assignement
		ASSIGN_BASIC,		// =
	};

	enum class unaryOperator
	{
		DEFAULT,
		LOGICNOT,		// ! 
		NEGATIVE,		// -
		POSITIVE		// +
	};

	namespace Operators
	{
		std::string toString(const binaryOperator& op);
		std::string toString(const unaryOperator& op);

		std::string getName(const binaryOperator& op);
		std::string getName(const unaryOperator& op);
	}

	class IdentifierInfo;

	// base expression 
	class Expr : public Stmt
	{
		public:
			Expr(const StmtKind& kind);
			inline virtual ~Expr() = 0 {}

			virtual bool isExpr() const override;
	};

	// A Null expression that's just a placeholder.
	// Mostly used by the parser to recover from situations, it doesn't necessarily
	// represent any language construction.
	class NullExpr : public Expr
	{
		public:
			NullExpr();
	};

	// Binary Expressions
	class BinaryExpr : public Expr
	{
		public:
			BinaryExpr(const binaryOperator &opt,std::unique_ptr<Expr> lhs = nullptr,std::unique_ptr<Expr> rhs = nullptr);

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
			UnaryExpr(const unaryOperator& opt,std::unique_ptr<Expr> node = nullptr);

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
			CastExpr(Type* castGoal,std::unique_ptr<Expr> child = nullptr);
			
			void setCastGoal(Type* goal);
			Type* getCastGoal();

			Expr* getChild();
			void setChild(std::unique_ptr<Expr> nc);
		private:
			Type* goal_ = nullptr;
			std::unique_ptr<Expr> child_;
	};

	// Literals
	class CharLiteralExpr : public Expr
	{
		public:
			CharLiteralExpr(const CharType &val);

			CharType getVal() const;
			void setVal(const CharType& val);
		private:
			CharType val_ = ' ';
	};

	class IntegerLiteralExpr : public Expr
	{
		public:
			IntegerLiteralExpr(const IntType &val);

			IntType getVal() const;
			void setVal(const IntType& val);
		private:
			IntType val_ = 0;
	};

	class FloatLiteralExpr : public Expr
	{
		public:
			FloatLiteralExpr(const FloatType &val);

			FloatType getVal() const;
			void setVal(const FloatType& val);
		private:
			FloatType val_ = 0.0f;
	};

	class StringLiteralExpr : public Expr
	{
		public:
			StringLiteralExpr(const std::string &val);

			std::string getVal() const;
			void setVal(const std::string& val);
		private:
			std::string val_ = "";
	};

	class BoolLiteralExpr : public Expr
	{
		public:
			BoolLiteralExpr(const bool &val);

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
		private:
			std::unique_ptr<ExprList> exprs_;
	};

	// Represents a reference to a declaration (namespace,variable,function) -> it's an identifier!
	class DeclRefExpr : public Expr
	{
		public:
			DeclRefExpr(IdentifierInfo * declid);
			
			IdentifierInfo * getIdentifier();
			void setDeclIdentifier(IdentifierInfo * id);
		private:
			IdentifierInfo * declId_;
	};

	// Represents a dot syntax "member of" expr.
	// eg : Fox.io, Fox.foo, etc
	class MemberOfExpr : public Expr
	{
		public:
			MemberOfExpr(std::unique_ptr<Expr> base = nullptr, IdentifierInfo *idInfo = nullptr);

			Expr* getBase();
			void setBase(std::unique_ptr<Expr> expr);

			IdentifierInfo* getMemberID();
			void setMemberName(IdentifierInfo* idInfo);
		private:
			std::unique_ptr<Expr> base_;
			IdentifierInfo *membName_;
	};

	// Arrays accesses : foo[0], etc.
	class ArrayAccessExpr : public Expr
	{
		public:
			ArrayAccessExpr(std::unique_ptr<Expr> expr, std::unique_ptr<Expr> idxexpr);

			void setBase(std::unique_ptr<Expr> expr);
			void setAccessIndexExpr(std::unique_ptr<Expr> expr);

			Expr* getBase() ;
			Expr* getAccessIndexExpr();
		private:
			// 2 Expr, the expression supposed to produce an array, and the expression contained within the square brackets that should produce the index.
			std::unique_ptr<Expr> base_;
			std::unique_ptr<Expr> accessIdxExpr_;
	};

	// Class Representing an Expression List.
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
			FunctionCallExpr(std::unique_ptr<Expr> base, std::unique_ptr<ExprList> elist = nullptr);

			Expr * getCallee() ;
			void setCallee(std::unique_ptr<Expr> base);

			ExprList* getExprList();
			void setExprList(std::unique_ptr<ExprList> elist);

		private:
			std::unique_ptr<Expr> callee_;
			std::unique_ptr<ExprList> args_;
	};

	// Parens Expr
	class ParensExpr : public Expr
	{
		public:
			ParensExpr(std::unique_ptr<Expr> expr, const SourceLoc& LParenLoc = SourceLoc(), const SourceLoc& RParenLoc = SourceLoc());

			SourceLoc getLeftParensLoc() const;
			SourceLoc getRightParensLoc() const;

			void setLeftParensLoc(const SourceLoc& sloc);
			void setRightParensLoc(const SourceLoc& sloc);

			Expr* getExpr();
			void setExpr(std::unique_ptr<Expr> expr);
		private:
			std::unique_ptr<Expr> expr_;
			SourceLoc RPLoc_, LPLoc_;
	};
}

