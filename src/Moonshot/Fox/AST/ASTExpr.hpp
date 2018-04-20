////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTExpr.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Declares the ASTExpr interface as well as derived nodes. 
////------------------------------------------------------////

#pragma once
#include "Moonshot/Fox/Basic/Typedefs.hpp"
#include "Moonshot/Fox/AST/ASTStmt.hpp"
#include "Moonshot/Fox/AST/Types.hpp"
#include "Moonshot/Fox/AST/Operators.hpp"
#include <memory>
#include <vector>

namespace Moonshot	
{
	class IVisitor;
	class IdentifierInfo;
	// base expression 
	class ASTExpr : public ASTStmt
	{
		public:
			ASTExpr() = default;
			inline virtual ~ASTExpr() = 0 {}
			virtual void accept(IVisitor& vis) = 0;
	};

	// A Null expression that's just a placeholder.
	// Mostly used by the parser to recover from situations, it doesn't necessarily
	// represent any language construction.
	class ASTNullExpr : public ASTExpr
	{
		public:
			ASTNullExpr() = default;
			virtual void accept(IVisitor &vis) override;
	};

	// Binary Expressions
	class ASTBinaryExpr : public ASTExpr
	{
		public:
			ASTBinaryExpr() = default;
			ASTBinaryExpr(const binaryOperator &opt,std::unique_ptr<ASTExpr> lhs = nullptr,std::unique_ptr<ASTExpr> rhs = nullptr);

			virtual void accept(IVisitor& vis) override;
			std::unique_ptr<ASTExpr> getSimple();	// If there is no right node and the optype is "pass", this will move and return the left node 

			ASTExpr* getLHS();
			ASTExpr* getRHS();

			void setLHS(std::unique_ptr<ASTExpr> nlhs);
			void setRHS(std::unique_ptr<ASTExpr> nrhs);

			binaryOperator getOp() const;
			void setOp(const binaryOperator& op);

			// Returns true if node has both a left_ and right_ child and op != default
			bool isComplete() const; 
		private:
			std::unique_ptr<ASTExpr> left_, right_;
			binaryOperator op_ = binaryOperator::DEFAULT;
	};

	// Unary Expressions
	class ASTUnaryExpr : public ASTExpr
	{
		public: 
			ASTUnaryExpr() = default;
			ASTUnaryExpr(const unaryOperator& opt,std::unique_ptr<ASTExpr> node = nullptr);
			virtual void accept(IVisitor& vis) override;

			ASTExpr* getChild();
			void setChild(std::unique_ptr<ASTExpr> nchild);

			unaryOperator getOp() const;
			void setOp(const unaryOperator& nop);

		private:
			std::unique_ptr<ASTExpr> child_;
			unaryOperator op_ = unaryOperator::DEFAULT;
	};

	// Explicit Cast Expressions
	class ASTCastExpr : public ASTExpr
	{
		public:
			ASTCastExpr() = default;
			ASTCastExpr(const Type* castGoal,std::unique_ptr<ASTExpr> child);
			
			virtual void accept(IVisitor& vis) override;

			void setCastGoal(const Type* goal);
			const Type* getCastGoal() const;

			ASTExpr* getChild();
			void setChild(std::unique_ptr<ASTExpr> nc);
		private:
			const Type* goal_ = nullptr;
			std::unique_ptr<ASTExpr> child_;
	};

	// Literals
	class ASTCharLiteralExpr : public ASTExpr
	{
		public:
			ASTCharLiteralExpr() = default;
			ASTCharLiteralExpr(const CharType &val);

			void accept(IVisitor &vis) override;

			CharType getVal() const;
			void setVal(const CharType& val);
		private:
			CharType val_ = ' ';
	};

	class ASTIntegerLiteralExpr : public ASTExpr
	{
		public:
			ASTIntegerLiteralExpr() = default;
			ASTIntegerLiteralExpr(const IntType &val);

			void accept(IVisitor &vis) override;

			IntType getVal() const;
			void setVal(const IntType& val);
		private:
			IntType val_ = 0;
	};

	class ASTFloatLiteralExpr : public ASTExpr
	{
		public:
			ASTFloatLiteralExpr() = default;
			ASTFloatLiteralExpr(const FloatType &val);

			void accept(IVisitor &vis) override;

			FloatType getVal() const;
			void setVal(const FloatType& val);
		private:
			FloatType val_ = 0.0f;
	};

	class ASTStringLiteralExpr : public ASTExpr
	{
		public:
			ASTStringLiteralExpr() = default;
			ASTStringLiteralExpr(const std::string &val);

			void accept(IVisitor &vis) override;

			std::string getVal() const;
			void setVal(const std::string& val);
		private:
			std::string val_ = "";
	};

	class ASTBoolLiteralExpr : public ASTExpr
	{
		public:
			ASTBoolLiteralExpr() = default;
			ASTBoolLiteralExpr(const bool &val);

			void accept(IVisitor &vis) override;

			bool getVal() const;
			void setVal(const bool& val);
		private:
			bool val_ = false;
	};

	class ExprList;
	// Array literals
	class ASTArrayLiteralExpr : public ASTExpr
	{
		public:
			ASTArrayLiteralExpr() = default;
			ASTArrayLiteralExpr(std::unique_ptr<ExprList> exprs);

			ExprList* getExprList();
			void setExprList(std::unique_ptr<ExprList> elist);
			bool hasExprList() const; 

			bool isEmpty() const;

			virtual void accept(IVisitor &vis);
		private:
			std::unique_ptr<ExprList> exprs_;
	};

	// interface for decl refs. Derived classes are references to a decl within this context (declref) and reference to member decls (memberref)
	class ASTDeclRef : public ASTExpr
	{
		// TODO After AST Upgrade/Rework
		// ASTDecl* getOriginalDecl();
		// void setDecl(ASTDecl* decl);
	};

	// Represents a reference to a declaration (namespace,variable,function) -> it's an identifier!
	class ASTDeclRefExpr : public ASTDeclRef
	{
		public:
			ASTDeclRefExpr() = default;
			ASTDeclRefExpr(IdentifierInfo * declid);

			void accept(IVisitor& vis) override;
			
			IdentifierInfo * getDeclIdentifier();
			void setDeclIdentifier(IdentifierInfo * id);
		private:
			IdentifierInfo * declId_;
	};

	// Represents a reference to a member : a namespace's, an object's field, etc.
	// expr is the expression that is being accessed, member_ is the declref to search.
		// For semantic analysis of this, we first check the base, and see if it produces a type "castable" do ASTDeclContext, if true, we do Semantic Analysis of member_
		// with the restricted context of the casted base_ result, if false, it's an error
	class ASTMemberAccessExpr : public ASTDeclRef
	{
		public:
			ASTMemberAccessExpr() = default;
			ASTMemberAccessExpr(std::unique_ptr<ASTExpr> base, std::unique_ptr<ASTDeclRef> memb);

			void accept(IVisitor& vis) override;

			ASTExpr* getBase();
			ASTDeclRef* getMemberDeclRef() const;

			void setBase(std::unique_ptr<ASTExpr> expr);
			void setMemberDeclRef(std::unique_ptr<ASTDeclRef> memb);
		private:
			// the expression that is being accessed
			std::unique_ptr<ASTExpr> base_;
			// the decl to search inside the expr
			std::unique_ptr<ASTDeclRef> member_;
	};

	class ASTArrayAccess : public ASTExpr
	{
		public:
			ASTArrayAccess(std::unique_ptr<ASTExpr> expr, std::unique_ptr<ASTExpr> idxexpr);
			void accept(IVisitor& vis) override;

			void setBase(std::unique_ptr<ASTExpr> expr);
			void setAccessIndexExpr(std::unique_ptr<ASTExpr> expr);

			ASTExpr* getBase() ;
			ASTExpr* getAccessIndexExpr();
		private:
			// 2 Expr, the expression supposed to produce an array, and the expression contained within the square brackets that should produce the index.
			std::unique_ptr<ASTExpr> base_;
			std::unique_ptr<ASTExpr> accessIdxExpr_;
	};

	// Node Representing an Expression List.
		// Note: This is not a "normal" node (not visitable!), it's more of a wrapper around a std::vector<std::unique_ptr<ASTExpr>>, so we can pass a list of 
		// expressions around easily.
	class ExprList
	{
		private:
			using ExprListTy = std::vector<std::unique_ptr<ASTExpr>>;

			using ExprListIter = ExprListTy::iterator;
			using ExprListIter_const = ExprListTy::const_iterator;
		public:
			ExprList() = default;

			void addExpr(std::unique_ptr<ASTExpr> expr);
			ASTExpr* getExpr(const std::size_t& ind);

			bool isEmpty() const;
			std::size_t size() const;

			ExprListIter begin();
			ExprListIter end();

			ExprListIter_const begin() const;
			ExprListIter_const end()const;
		private:
			ExprListTy exprs_;
	};

	// Function calls
	class ASTFunctionCallExpr : public ASTDeclRef
	{
		public:
			ASTFunctionCallExpr() = default;

			IdentifierInfo * getFunctionIdentifier() ;
			void setFunctionIdentifier(IdentifierInfo * fnId);

			ExprList* getExprList();
			void setExprList(std::unique_ptr<ExprList> elist);

			void accept(IVisitor& vis) override;
		private:
			// the Function's name
			IdentifierInfo * fnId_;
			// it's args
			std::unique_ptr<ExprList> args_;
	};
}

