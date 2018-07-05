////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Expr.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Declares the Expr interface as well as derived nodes. 
////------------------------------------------------------////

#pragma once
#include "Fox/Common/Typedefs.hpp"
#include "Fox/AST/Stmt.hpp"
#include "Fox/AST/Type.hpp"

namespace fox	
{
	// operators Enums & Dictionaries
	enum class BinaryOperator
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

	enum class UnaryOperator
	{
		DEFAULT,
		LOGICNOT,		// ! 
		NEGATIVE,		// -
		POSITIVE		// +
	};

	namespace operators
	{
		std::string toString(const BinaryOperator& op);
		std::string toString(const UnaryOperator& op);

		std::string getName(const BinaryOperator& op);
		std::string getName(const UnaryOperator& op);
	}

	class IdentifierInfo;

	// base expression 
	class Expr : public Stmt
	{
		protected:
			Expr(StmtKind kind, const SourceLoc& begLoc, const SourceLoc& endLoc);
	};

	// Binary Expressions
	class BinaryExpr : public Expr
	{
		public:
			BinaryExpr();
			BinaryExpr(BinaryOperator opt,std::unique_ptr<Expr> lhs,std::unique_ptr<Expr> rhs, const SourceLoc& begLoc, const SourceRange& opRange, const SourceLoc& endLoc);

			Expr* getLHS();
			Expr* getRHS();

			const Expr* getLHS() const;
			const Expr* getRHS() const;

			void setLHS(std::unique_ptr<Expr> nlhs);
			void setRHS(std::unique_ptr<Expr> nrhs);

			BinaryOperator getOp() const;
			void setOp(const BinaryOperator& op);

			SourceRange getOpRange() const;
		private:
			SourceRange opRange_;
			std::unique_ptr<Expr> left_, right_;
			BinaryOperator op_ = BinaryOperator::DEFAULT;
	};

	// Unary Expressions
	class UnaryExpr : public Expr
	{
		public: 
			UnaryExpr();
			UnaryExpr(UnaryOperator opt,std::unique_ptr<Expr> node, const SourceLoc& begLoc, const SourceRange& opRange, const SourceLoc& endLoc);

			Expr* getChild();
			const Expr* getChild() const;
			void setChild(std::unique_ptr<Expr> nchild);

			UnaryOperator getOp() const;
			void setOp(const UnaryOperator& nop);

			SourceRange getOpRange() const;
		private:
			SourceRange opRange_;
			std::unique_ptr<Expr> child_;
			UnaryOperator op_ = UnaryOperator::DEFAULT;
	};

	// Explicit Cast Expressions
	class CastExpr : public Expr
	{
		public:
			CastExpr();
			CastExpr(Type* castGoal,std::unique_ptr<Expr> child, const SourceLoc& begLoc, const SourceRange& typeRange, const SourceLoc& endLoc);
			
			void setCastGoal(Type* goal);
			Type* getCastGoal();
			const Type* getCastGoal() const;

			Expr* getChild();
			const  Expr* getChild() const;
			void setChild(std::unique_ptr<Expr> nc);

			SourceRange getTypeRange() const;
		private:
			SourceRange typeRange_;
			Type* goal_ = nullptr;
			std::unique_ptr<Expr> child_;
	};

	// Literals
	class CharLiteralExpr : public Expr
	{
		public:
			CharLiteralExpr();
			CharLiteralExpr(CharType val,const SourceLoc& begLoc, const SourceLoc& endLoc);

			CharType getVal() const;
			void setVal(const CharType& val);
		private:
			CharType val_ = ' ';
	};

	class IntegerLiteralExpr : public Expr
	{
		public:
			IntegerLiteralExpr();
			IntegerLiteralExpr(IntType val, const SourceLoc& begLoc, const SourceLoc& endLoc);

			IntType getVal() const;
			void setVal(const IntType& val);
		private:
			IntType val_ = 0;
	};

	class FloatLiteralExpr : public Expr
	{
		public:
			FloatLiteralExpr();
			FloatLiteralExpr(FloatType val, const SourceLoc& begLoc, const SourceLoc& endLoc);

			FloatType getVal() const;
			void setVal(const FloatType& val);
		private:
			FloatType val_ = 0.0f;
	};

	class StringLiteralExpr : public Expr
	{
		public:
			StringLiteralExpr();
			StringLiteralExpr(const std::string& val, const SourceLoc& begLoc, const SourceLoc& endLoc);

			std::string getVal() const;
			void setVal(const std::string& val);
		private:
			std::string val_ = "";
	};

	class BoolLiteralExpr : public Expr
	{
		public:
			BoolLiteralExpr();
			BoolLiteralExpr(bool val, const SourceLoc& begLoc, const SourceLoc& endLoc);

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
			ArrayLiteralExpr();
			ArrayLiteralExpr(std::unique_ptr<ExprList> exprs, const SourceLoc& begLoc, const SourceLoc& endLoc);

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
			DeclRefExpr();
			DeclRefExpr(IdentifierInfo * declid, const SourceLoc& begLoc, const SourceLoc& endLoc);
			
			IdentifierInfo * getIdentifier();
			const IdentifierInfo * getIdentifier() const;
			void setDeclIdentifier(IdentifierInfo * id);
		private:
			IdentifierInfo * declId_;
	};

	// Represents a dot syntax "member of" expr.
	// eg : Fox.io, Fox.foo, etc
	class MemberOfExpr : public Expr
	{
		public:
			MemberOfExpr();
			MemberOfExpr(std::unique_ptr<Expr> base, IdentifierInfo *idInfo, const SourceLoc& begLoc, const SourceLoc& dotLoc, const SourceLoc& endLoc);

			Expr* getBase();
			const Expr* getBase() const;
			void setBase(std::unique_ptr<Expr> expr);

			IdentifierInfo* getMemberID();
			const IdentifierInfo* getMemberID() const;
			void setMemberName(IdentifierInfo* idInfo);

			SourceLoc getDotLoc() const;
		private:
			SourceLoc dotLoc_;
			std::unique_ptr<Expr> base_;
			IdentifierInfo *membName_;
	};

	// Arrays accesses : foo[0], etc.
	class ArrayAccessExpr : public Expr
	{
		public:
			ArrayAccessExpr();
			ArrayAccessExpr(std::unique_ptr<Expr> expr, std::unique_ptr<Expr> idxexpr, const SourceLoc& begLoc, const SourceLoc& endLoc);

			void setBase(std::unique_ptr<Expr> expr);
			void setAccessIndexExpr(std::unique_ptr<Expr> expr);

			Expr* getBase() ;
			Expr* getAccessIndexExpr();

			const Expr* getBase() const;
			const Expr* getAccessIndexExpr() const;
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
			Expr* getExpr(std::size_t ind);
			const Expr* getExpr(std::size_t ind) const;

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
			FunctionCallExpr();
			FunctionCallExpr(std::unique_ptr<Expr> base, std::unique_ptr<ExprList> elist, const SourceLoc& begLoc, const SourceLoc& endLoc);
			
			Expr * getCallee();
			const Expr * getCallee() const;
			void setCallee(std::unique_ptr<Expr> base);

			ExprList* getExprList();
			const ExprList* getExprList() const;
			void setExprList(std::unique_ptr<ExprList> elist);
		private:
			std::unique_ptr<Expr> callee_;
			std::unique_ptr<ExprList> args_;
	};

	// Parens Expr
	class ParensExpr : public Expr
	{
		public:
			ParensExpr();
			ParensExpr(std::unique_ptr<Expr> expr, const SourceLoc& begLoc, const SourceLoc& endLoc);

			Expr* getExpr();
			const Expr* getExpr() const;
			void setExpr(std::unique_ptr<Expr> expr);
		private:
			std::unique_ptr<Expr> expr_;
	};
}

