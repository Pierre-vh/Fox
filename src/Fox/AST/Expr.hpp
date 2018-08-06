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

			SourceRange getRange() const;

			SourceLoc getBegLoc() const;
			SourceLoc getEndLoc() const;

			// Prohibit the use of builtin placement new & delete
			void *operator new(std::size_t) throw() = delete;
			void operator delete(void *) throw() = delete;
			void* operator new(std::size_t, void*) = delete;

			// Only allow allocation through the ASTContext
			void* operator new(std::size_t sz, ASTContext &ctxt, std::uint8_t align = alignof(Expr));

			// Companion operator delete to silence C4291 on MSVC
			void operator delete(void*, ASTContext&, std::uint8_t) {}

		protected:
			Expr(ExprKind kind, const SourceLoc& begLoc, const SourceLoc& endLoc);

		private:
			const ExprKind kind_;
			SourceRange range_;
	};

	// A Vector of Pointers to Expressions
	using ExprVector = std::vector<Expr*>;

	// BinaryExpr
	//		A binary expression: 2+2
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
				const SourceLoc& begLoc, const SourceRange& opRange, const SourceLoc& endLoc);

			Expr* getLHS() const;
			Expr* getRHS() const;

			void setLHS(Expr* expr);
			void setRHS(Expr* expr);

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
			UnaryExpr(OpKind op, Expr* node, const SourceLoc& begLoc, const SourceRange& opRange, const SourceLoc& endLoc);

			Expr* getExpr() const;
			void setExpr(Expr* expr);

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
			CastExpr(Type* castGoal, Expr* expr, const SourceLoc& begLoc, const SourceRange& typeRange, const SourceLoc& endLoc);
			
			Type* getCastGoal() const;
			void setCastGoal(Type* goal);

			Expr* getExpr() const;
			void setExpr(Expr* expr);

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
			CharLiteralExpr(CharType val,const SourceLoc& begLoc, const SourceLoc& endLoc);

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
			IntegerLiteralExpr(IntType val, const SourceLoc& begLoc, const SourceLoc& endLoc);

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
			FloatLiteralExpr(FloatType val, const SourceLoc& begLoc, const SourceLoc& endLoc);

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
			StringLiteralExpr(const std::string& val, const SourceLoc& begLoc, const SourceLoc& endLoc);

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
			BoolLiteralExpr(bool val, const SourceLoc& begLoc, const SourceLoc& endLoc);

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
			ArrayLiteralExpr(ExprVector&& exprs, const SourceLoc& begLoc, const SourceLoc& endLoc);

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
			DeclRefExpr(IdentifierInfo * declid, const SourceLoc& begLoc, const SourceLoc& endLoc);
			
			IdentifierInfo* getIdentifier() const;
			void setDeclIdentifier(IdentifierInfo * id);

			static bool classof(const Expr* expr)
			{
				return (expr->getKind() == ExprKind::DeclRefExpr);
			}

		private:
			IdentifierInfo * declId_ = nullptr;
	};

	// MemberOfExpr
	//		A member access : foo.bar
	class MemberOfExpr : public Expr
	{
		public:
			MemberOfExpr();
			MemberOfExpr(Expr* base, IdentifierInfo *idInfo, 
				const SourceLoc& begLoc, const SourceLoc& dotLoc, const SourceLoc& endLoc);

			Expr* getBase() const;
			void setBase(Expr* expr);

			IdentifierInfo* getMemberID() const;
			void setMemberName(IdentifierInfo* idInfo);

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
			ArrayAccessExpr(Expr* base, Expr* idx, const SourceLoc& begLoc, const SourceLoc& endLoc);

			void setBase(Expr* expr);
			void setAccessIndexExpr(Expr* expr);

			Expr* getBase() const;
			Expr* getAccessIndexExpr() const;

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
			FunctionCallExpr(Expr* callee, ExprVector&& args, const SourceLoc& begLoc, const SourceLoc& endLoc);
			
			Expr* getCallee() const;
			void setCallee(Expr* base);

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
			ParensExpr(Expr* expr, const SourceLoc& begLoc, const SourceLoc& endLoc);

			Expr* getExpr() const;
			void setExpr(Expr* expr);

			static bool classof(const Expr* expr)
			{
				return (expr->getKind() == ExprKind::ParensExpr);
			}

		private:
			Expr* expr_ = nullptr;
	};
}

