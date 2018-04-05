////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTExpr.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Declares the IASTExpr interface as well as derived nodes. 
////------------------------------------------------------////

#pragma once

#include "Moonshot/Fox/AST/ASTStmt.hpp"
#include "Moonshot/Fox/AST/Operators.hpp"			// enums
#include "Moonshot/Common/Types/Types.hpp"		// FoxValue
#include <memory>
#include <vector>
#include <functional>

namespace Moonshot	
{
	class IVisitor;

	// base expression 
	struct IASTExpr : public IASTStmt
	{
		public:
			IASTExpr() = default;
			inline virtual ~IASTExpr() = 0 {}
			virtual void accept(IVisitor& vis) = 0;

			FoxType getResultType() const;
			void setResultType(const FoxType& ft);
		protected:
			FoxType resultType_ = 0; // The planified result type of the expression after execution. this is set by the typechecker.
	};

	// Binary Expressions
	struct ASTBinaryExpr : public IASTExpr
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
	struct ASTUnaryExpr : public IASTExpr
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
	struct ASTCastExpr : public IASTExpr
	{
		public:
			ASTCastExpr() = default;
			ASTCastExpr(const FoxType& castGoal,std::unique_ptr<IASTExpr> ch = nullptr);
			virtual void accept(IVisitor& vis) override;

			void setCastGoal(const FoxType& ncg);		// castgoal is stored inside
			FoxType getCastGoal() const; 

			IASTExpr* getChild();
			void setChild(std::unique_ptr<IASTExpr> nc);
		private:
			std::unique_ptr<IASTExpr> child_;
	};

	// Literals
	struct ASTLiteralExpr : public IASTExpr 
	{
		public:
			ASTLiteralExpr() = default;
			ASTLiteralExpr(const FoxValue &fv);

			void accept(IVisitor& vis) override;

			FoxValue getVal() const;
			void setVal(const FoxValue& nval);

		private:
			FoxValue val_;
	};

	/*
		Note: the AST needs to be adapted for theses 2 nodes. std::strings should be replaced by a pointer to a IASTIdentifier (or find a better name)
		IASTIdentifier has 2 children : Unresolved ID and resolved ID. Unresolved ID are the raw strings produced by the parsing, and they're replaced by the 
		Resolver in the Semantic analysis phase by Resolved IDs. Resolved IDs contain a pointer to an entry in the master symbols table.
	*/

	// interface for decl refs. Derived classes are references to a decl within this context (declref) and reference to member decls (memberref)
	struct IASTDeclRef : public IASTExpr
	{
		// TODO After AST Upgrade/Rework
		// ASTDecl* getOriginalDecl();
		// void setDecl(ASTDecl* decl);
	};

	// Represents a reference to a declaration (namespace,variable,function) -> it's an identifier!
	struct ASTDeclRefExpr : public IASTDeclRef
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
	struct ASTMemberAccessExpr : public IASTDeclRef
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

	struct ASTArrayAccess : public IASTDeclRef
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
	struct ExprList
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
	struct ASTFunctionCallExpr : public IASTDeclRef
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

