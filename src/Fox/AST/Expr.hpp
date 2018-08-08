////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Expr.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file contains the Expr Hierarchy.
////------------------------------------------------------////

#pragma once

#include "Fox/Common/Typedefs.hpp"
#include "Fox/AST/Type.hpp"
#include "Fox/Common/Source.hpp"
#include <vector>

namespace fox	
{
	// Kinds of Expressions
	enum class ExprKind: std::uint8_t
	{
		#define EXPR(ID,PARENT) ID,
		#include "ExprNodes.def"
	};

	// Forward Declarations
	class IdentifierInfo;
	class ASTContext;

	// Expr
	//		Common base class for every expression
	class Expr
	{
		public:
			ExprKind getKind() const;

			void setRange(const SourceRange& range);
			SourceRange getRange() const;

			// Prohibit the use of builtin placement new & delete
			void *operator new(std::size_t) throw() = delete;
			void operator delete(void *) throw() = delete;
			void* operator new(std::size_t, void*) = delete;

			// Only allow allocation through the ASTContext
			void* operator new(std::size_t sz, ASTContext &ctxt, std::uint8_t align = alignof(Expr));

			// Companion operator delete to silence C4291 on MSVC
			void operator delete(void*, ASTContext&, std::uint8_t) {}

		protected:
			Expr(ExprKind kind, const SourceRange& range);

		private:
			const ExprKind kind_;
			SourceRange range_;
	};

	// A Vector of Pointers to Expressions
	using ExprVector = std::vector<Expr*>;

	// BinaryExpr
	//		A binary expression
	class BinaryExpr : public Expr
	{
		public:
			enum class OpKind: std::uint8_t
			{
				#define BINARY_OP(ID, SIGN, NAME) ID,
				#include "Operators.def"
			};

			BinaryExpr();
			BinaryExpr(OpKind op, Expr* lhs, Expr* rhs, 
				const SourceRange& range, const SourceRange& opRange);

			void setLHS(Expr* expr);
			Expr* getLHS();
			const Expr* getLHS() const;

			void setRHS(Expr* expr);
			Expr* getRHS();
			const Expr* getRHS() const;

			OpKind getOp() const;
			void setOp(OpKind op);

			SourceRange getOpRange() const;

			static bool classof(const Expr* expr)
			{
				return (expr->getKind() == ExprKind::BinaryExpr);
			}

			// Get information about an operator as a string
			static std::string getOpSign(OpKind op);
			static std::string getOpName(OpKind op);
			static std::string getOpID(OpKind op);

		private:
			SourceRange opRange_;
			Expr* lhs_ = nullptr;
			Expr* rhs_ = nullptr;
			OpKind op_ = OpKind::Invalid;
	};

	// UnaryExpr
	//		A unary expression: -2
	class UnaryExpr : public Expr
	{
		public: 
			enum class OpKind: std::uint8_t
			{
				#define UNARY_OP(ID, SIGN, NAME) ID,
				#include "Operators.def"
			};

			UnaryExpr();
			UnaryExpr(OpKind op, Expr* node, const SourceRange& range,
				const SourceRange& opRange);
			
			void setExpr(Expr* expr);
			Expr* getExpr();
			const Expr* getExpr() const;

			OpKind getOp() const;
			void setOp(OpKind op);

			SourceRange getOpRange() const;

			static bool classof(const Expr* expr)
			{
				return (expr->getKind() == ExprKind::UnaryExpr);
			}

			// Get information about an operator as a string
			static std::string getOpSign(OpKind op);
			static std::string getOpName(OpKind op);
			static std::string getOpID(OpKind op);

		private:
			SourceRange opRange_;
			Expr* expr_ = nullptr;
			OpKind op_ = OpKind::Invalid;
	};

	// CastExpr
	//		An explicit "as" cast expression: foo as int
	class CastExpr : public Expr
	{
		public:
			CastExpr();
			CastExpr(Type* castGoal, Expr* expr, const SourceRange& range, 
				const SourceRange& typeRange);
			
			void setCastGoal(Type* goal);
			Type* getCastGoal();
			const Type* getCastGoal() const;

			void setExpr(Expr* expr);
			Expr* getExpr();
			const Expr* getExpr() const;

			SourceRange getTypeRange() const;

			static bool classof(const Expr* expr)
			{
				return (expr->getKind() == ExprKind::CastExpr);
			}

		private:
			SourceRange typeRange_;
			Type* goal_ = nullptr;
			Expr* expr_ = nullptr;
	};

	// CharLiteralExpr
	//		A char literal: 'a'
	class CharLiteralExpr : public Expr
	{
		public:
			CharLiteralExpr();
			CharLiteralExpr(CharType val, const SourceRange& range);

			CharType getVal() const;
			void setVal(CharType val);

			static bool classof(const Expr* expr)
			{
				return (expr->getKind() == ExprKind::CharLiteralExpr);
			}

		private:
			CharType val_ = ' ';
	};

	// IntegerLiteralExpr
	//		An integer literal: 2
	class IntegerLiteralExpr : public Expr
	{
		public:
			IntegerLiteralExpr();
			IntegerLiteralExpr(IntType val, const SourceRange& range);

			IntType getVal() const;
			void setVal(IntType val);

			static bool classof(const Expr* expr)
			{
				return (expr->getKind() == ExprKind::IntegerLiteralExpr);
			}

		private:
			IntType val_ = 0;
	};

	// FloatLiteralExpr
	//		A float literal: 3.14
	class FloatLiteralExpr : public Expr
	{
		public:
			FloatLiteralExpr();
			FloatLiteralExpr(FloatType val, const SourceRange& range);

			FloatType getVal() const;
			void setVal(FloatType val);

			static bool classof(const Expr* expr)
			{
				return (expr->getKind() == ExprKind::FloatLiteralExpr);
			}

		private:
			FloatType val_ = 0.0f;
	};

	// StringLiteralExpr
	//		A string literal: "foo"
	class StringLiteralExpr : public Expr
	{
		public:
			StringLiteralExpr();
			StringLiteralExpr(const std::string& val, const SourceRange& range);

			std::string getVal() const;
			void setVal(const std::string& val);

			static bool classof(const Expr* expr)
			{
				return (expr->getKind() == ExprKind::StringLiteralExpr);
			}

		private:
			std::string val_ = "";
	};

	// BoolLiteralExpr
	//		true/false boolean literal
	class BoolLiteralExpr : public Expr
	{
		public:
			BoolLiteralExpr();
			BoolLiteralExpr(bool val, const SourceRange& range);

			bool getVal() const;
			void setVal(bool val);

			static bool classof(const Expr* expr)
			{
				return (expr->getKind() == ExprKind::BoolLiteralExpr);
			}

		private:
			bool val_ = false;
	};

	// ArrayLiteralExpr
	//		An array literal: [1, 2, 3]
	class ArrayLiteralExpr : public Expr
	{
		public:
			ArrayLiteralExpr();
			ArrayLiteralExpr(const ExprVector& exprs, const SourceRange& range);

			ExprVector& getExprs();
			const ExprVector& getExprs() const;
			void setExprs(ExprVector&& elist);

			std::size_t getSize() const;
			bool isEmpty() const;

			static bool classof(const Expr* expr)
			{
				return (expr->getKind() == ExprKind::ArrayLiteralExpr);
			}

		private:
			ExprVector exprs_;
	};

	// DeclRefExpr
	//		A identifier that references a declaration: foo
	class DeclRefExpr : public Expr
	{
		public:
			DeclRefExpr();
			DeclRefExpr(IdentifierInfo* declid, const SourceRange& range);

			void setIdentifier(IdentifierInfo * id);
			IdentifierInfo* getIdentifier();
			const IdentifierInfo* getIdentifier() const;

			static bool classof(const Expr* expr)
			{
				return (expr->getKind() == ExprKind::DeclRefExpr);
			}

		private:
			IdentifierInfo* id_ = nullptr;
	};

	// MemberOfExpr
	//		A member access : foo.bar
	class MemberOfExpr : public Expr
	{
		public:
			MemberOfExpr();
			MemberOfExpr(Expr* base, IdentifierInfo *idInfo, 
				const SourceRange& range, const SourceLoc& dotLoc);

			void setBase(Expr* expr);
			Expr* getBase();
			const Expr* getBase() const;

			void setMemberID(IdentifierInfo* idInfo);
			IdentifierInfo* getMemberID();
			const IdentifierInfo* getMemberID() const;

			SourceLoc getDotLoc() const;

			static bool classof(const Expr* expr)
			{
				return (expr->getKind() == ExprKind::MemberOfExpr);
			}

		private:
			SourceLoc dotLoc_;
			Expr* base_ = nullptr;
			IdentifierInfo *membName_ = nullptr;
	};

	// ArrayAccessExpr
	//		Array access (or subscript): foo[3]
	class ArrayAccessExpr : public Expr
	{
		public:
			ArrayAccessExpr();
			ArrayAccessExpr(Expr* base, Expr* idx, const SourceRange& range);

			void setBase(Expr* expr);
			Expr* getBase();
			const Expr* getBase() const;

			void setAccessIndexExpr(Expr* expr);
			Expr* getAccessIndexExpr();
			const Expr* getAccessIndexExpr() const;

			static bool classof(const Expr* expr)
			{
				return (expr->getKind() == ExprKind::ArrayAccessExpr);
			}

		private:
			Expr* base_ = nullptr;
			Expr* idxExpr_ = nullptr;
	};

	// FunctionCallExpr
	//		A function call: foo(3.14)
	class FunctionCallExpr : public Expr
	{
		public:
			FunctionCallExpr();
			FunctionCallExpr(Expr* callee, const ExprVector& args, const SourceRange& range);
			
			void setCallee(Expr* base);
			Expr* getCallee();
			const Expr* getCallee() const;

			ExprVector& getArgs();
			const ExprVector& getArgs() const;
			void setArgs(ExprVector&& exprs);

			ExprVector::iterator args_begin();
			ExprVector::const_iterator args_begin() const;

			ExprVector::iterator args_end();
			ExprVector::const_iterator args_end() const;

			static bool classof(const Expr* expr)
			{
				return (expr->getKind() == ExprKind::FunctionCallExpr);
			}

		private:
			Expr* callee_ = nullptr;
			ExprVector args_;
	};

	// ParensExpr 
	//		An expression in round brackets: (2+2)
	class ParensExpr : public Expr
	{
		public:
			ParensExpr();
			ParensExpr(Expr* expr, const SourceRange& range);

			void setExpr(Expr* expr);
			Expr* getExpr();
			const Expr* getExpr() const;

			static bool classof(const Expr* expr)
			{
				return (expr->getKind() == ExprKind::ParensExpr);
			}

		private:
			Expr* expr_ = nullptr;
	};
}

