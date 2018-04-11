////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTExpr.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Declares the IASTExpr interface as well as derived nodes. 
////------------------------------------------------------////

#pragma once
#include "Moonshot/Common/Types/Types.hpp"
#include "Moonshot/Fox/AST/ASTStmt.hpp"
#include "Moonshot/Fox/AST/Types.hpp"
#include "Moonshot/Fox/AST/Operators.hpp"			// enums
#include <memory>
#include <vector>
#include <functional>

namespace Moonshot	
{
	class IVisitor;

	// base expression 
	class IASTExpr : public IASTStmt
	{
		public:
			IASTExpr() = default;
			inline virtual ~IASTExpr() = 0 {}
			virtual void accept(IVisitor& vis) = 0;
	};

	// Binary Expressions
	class ASTBinaryExpr : public IASTExpr
	{
		public:
			ASTBinaryExpr() = default;
			ASTBinaryExpr(const binaryOperator &opt,std::unique_ptr<IASTExpr> lhs = nullptr,std::unique_ptr<IASTExpr> rhs = nullptr);

			virtual void accept(IVisitor& vis) override;
			std::unique_ptr<IASTExpr> getSimple();	// If there is no right node and the optype is "pass", this will move and return the left node 

			IASTExpr* getLHS();
			IASTExpr* getRHS();

			void setLHS(std::unique_ptr<IASTExpr> nlhs);
			void setRHS(std::unique_ptr<IASTExpr> nrhs);

			binaryOperator getOp() const;
			void setOp(const binaryOperator& op);

			// Returns true if node has both a left_ and right_ child and op != default
			bool isComplete() const; 
		private:
			std::unique_ptr<IASTExpr> left_, right_;
			binaryOperator op_ = binaryOperator::DEFAULT;
	};

	// Unary Expressions
	class ASTUnaryExpr : public IASTExpr
	{
		public: 
			ASTUnaryExpr() = default;
			ASTUnaryExpr(const unaryOperator& opt,std::unique_ptr<IASTExpr> node = nullptr);
			virtual void accept(IVisitor& vis) override;

			IASTExpr* getChild();
			void setChild(std::unique_ptr<IASTExpr> nchild);

			unaryOperator getOp() const;
			void setOp(const unaryOperator& nop);

		private:
			std::unique_ptr<IASTExpr> child_;
			unaryOperator op_ = unaryOperator::DEFAULT;
	};

	// Explicit Cast Expressions
	class ASTCastExpr : public IASTExpr
	{
		public:
			ASTCastExpr() = default;
			ASTCastExpr(IType* castGoal,std::unique_ptr<IASTExpr> ch = nullptr);
			
			virtual void accept(IVisitor& vis) override;

			void setCastGoal(IType* goal);
			IType* getCastGoal();

			IASTExpr* getChild();
			void setChild(std::unique_ptr<IASTExpr> nc);
		private:
			IType * goal_ = nullptr;
			std::unique_ptr<IASTExpr> child_;
	};

	// Literals
	class ASTCharLiteralExpr : public IASTExpr
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

	class ASTIntegerLiteralExpr : public IASTExpr
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

	class ASTFloatLiteralExpr : public IASTExpr
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

	class ASTStringLiteralExpr : public IASTExpr
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

	class ASTBoolLiteralExpr : public IASTExpr
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

	// interface for decl refs. Derived classes are references to a decl within this context (declref) and reference to member decls (memberref)
	class IASTDeclRef : public IASTExpr
	{
		// TODO After AST Upgrade/Rework
		// ASTDecl* getOriginalDecl();
		// void setDecl(ASTDecl* decl);
	};

	// Represents a reference to a declaration (namespace,variable,function) -> it's an identifier!
	class ASTDeclRefExpr : public IASTDeclRef
	{
		public:
			ASTDeclRefExpr() = default;
			ASTDeclRefExpr(const std::string& vname);

			void accept(IVisitor& vis) override;
			
			std::string getDeclnameStr() const;
			void setDeclnameStr(const std::string& str);
		private:
			std::string declname_ = "";
	};

	// Represents a reference to a member : a namespace's, an object's field, etc.
	// expr is the expression that is being accessed, id_ is the identifier to search.
	class ASTMemberAccessExpr : public IASTDeclRef
	{
		public:
			ASTMemberAccessExpr() = default;
			ASTMemberAccessExpr(std::unique_ptr<IASTExpr> base, std::unique_ptr<IASTDeclRef> memb);

			void accept(IVisitor& vis) override;

			IASTExpr* getBase();
			IASTDeclRef* getMemberDeclRef() const;

			void setBase(std::unique_ptr<IASTExpr> expr);
			void setMemberDeclRef(std::unique_ptr<IASTDeclRef> memb);
		private:
			// the expression that is being accessed
			std::unique_ptr<IASTExpr> base_;
			// the decl to search inside the expr
			std::unique_ptr<IASTDeclRef> member_;
	};

	class ASTArrayAccess : public IASTDeclRef
	{
		public:
			ASTArrayAccess(std::unique_ptr<IASTExpr> expr, std::unique_ptr<IASTExpr> idxexpr);
			void accept(IVisitor& vis) override;

			void setBase(std::unique_ptr<IASTExpr> expr);
			void setAccessIndexExpr(std::unique_ptr<IASTExpr> expr);

			IASTExpr* getBase() ;
			IASTExpr* getAccessIndexExpr();
		private:
			// 2 Expr, the expression supposed to produce an array, and the expression contained within the square brackets that should produce the index.
			std::unique_ptr<IASTExpr> base_;
			std::unique_ptr<IASTExpr> accessIdxExpr_;
	};

	// Node/Helper struct that's a wrapper around a std::vector of std::unique_ptr to <IASTExpr>.
	// used by function call nodes and the parser.
	class ExprList
	{
		private:
			using expr_iter = std::vector<std::unique_ptr<IASTExpr>>::iterator;
		public:
			ExprList() = default;

			void addExpr(std::unique_ptr<IASTExpr> expr);
			const IASTExpr* getExpr(const std::size_t& ind);

			bool isEmpty() const;
			std::size_t size() const;

			expr_iter exprList_beg();
			expr_iter exprList_end();

			void iterate(std::function<void(IASTExpr*)> fn);
		private:
			std::vector<std::unique_ptr<IASTExpr>> exprs_;
	};

	// Function calls
	class ASTFunctionCallExpr : public IASTDeclRef
	{
		public:
			ASTFunctionCallExpr() = default;

			std::string getFunctionName() const;
			ExprList* getExprList();

			void setExprList(std::unique_ptr<ExprList> elist);
			void setFunctionName(const std::string& fnname);

			void accept(IVisitor& vis) override;
		private:
			// the Function's name
			std::string funcname_;
			// it's args
			std::unique_ptr<ExprList> args_;
	};
}

