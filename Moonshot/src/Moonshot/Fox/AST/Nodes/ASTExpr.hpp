////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTExpr.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Declares the IASTExpr interface as well as derived nodes. 
////------------------------------------------------------////

#pragma once

#include "ASTStmt.hpp"
#include "Moonshot/Fox/AST/IVisitor.hpp"
#include "Moonshot/Fox/Common/Operators.hpp"			// enums
#include "Moonshot/Common/Types/Types.hpp"		// FoxValue
#include <memory>
#include <vector>

namespace Moonshot	
{
	// base expression interface
	struct IASTExpr : public IASTStmt
	{
		public:
			IASTExpr() = default;
			inline virtual ~IASTExpr() = 0 {}
			virtual void accept(IVisitor& vis) = 0;
			FoxType resultType_ = 0; // The planified result type of the expression after execution. this is set by the typechecker.
	};

	// Binary Expressions
	struct ASTBinaryExpr : public IASTExpr
	{
		public:
			ASTBinaryExpr() = default;
			ASTBinaryExpr(const binaryOperator &opt);


			std::unique_ptr<IASTExpr> left_, right_;
			binaryOperator op_ = binaryOperator::DEFAULT;

			virtual void accept(IVisitor& vis) override;
			std::unique_ptr<IASTExpr> getSimple();	// If there is no right node and the optype is "pass", this will move and return the left node 
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
			ASTCastExpr(const FoxType& castGoal);
			virtual void accept(IVisitor& vis) override;

			std::unique_ptr<IASTExpr> child_;

			void setCastGoal(const FoxType& ncg);		// castgoal is stored inside
			FoxType getCastGoal() const; 
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
	struct IASTDeclRef : public IASTExpr {};

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
	struct ASTMemberOfExpr : public IASTDeclRef
	{
		public:
			ASTMemberOfExpr() = default;
			ASTMemberOfExpr(std::unique_ptr<IASTExpr> base, const std::string& membname);

			void accept(IVisitor& vis) override;

			IASTExpr* getBase();
			std::string getMemberNameStr() const;

			void setBase(std::unique_ptr<IASTExpr> expr);
			void setDeclname(const std::string& membname);
		private:
			// the expression that is being accessed
			std::unique_ptr<IASTExpr> base_;
			// the decl to search inside the expr
			std::string memb_name_;
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
		private:
			std::vector<std::unique_ptr<IASTExpr>> exprs_;
	};

	// Function calls
	struct ASTFunctionCallExpr : public IASTExpr
	{
		public:
			ASTFunctionCallExpr() = default;

			IASTDeclRef* getDeclRefExpr();
			ExprList* getExprList();

			void setExprList(std::unique_ptr<ExprList> elist);
			void setDeclRef(std::unique_ptr<IASTDeclRef> dref);

			void accept(IVisitor& vis) override;
		private:
			// the fn
			std::unique_ptr<IASTDeclRef> declref_;
			// it's args
			std::unique_ptr<ExprList> args_;
	};
}

