////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTVisitor.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file implements the "Visitor" class, which dispatchs Stmt,Decls and Types
// to their appropriate "visit" method.
////------------------------------------------------------////

#pragma once

#include "Decl.hpp"
#include "Stmt.hpp"
#include "Expr.hpp"
#include "Type.hpp"
#include "ASTNode.hpp"
#include "Fox/Common/Errors.hpp"
#include <utility>

namespace fox
{
	// Visitor class, which takes a few templates arguments : The derived class, The return type and the Args
	// that should be passed to the Visit method.
	template<
		typename Derived, 
		typename DeclRtrTy,
		typename ExprRtrTy,
		typename StmtRtrTy,
		typename TypeRtrTy,
		typename ... Args
	>
	class ASTVisitor
	{
		public:
			// Visit ASTNode
			void visit(ASTNode node, Args... args)
			{
				if (node.is<Decl>())
					visit(node.get<Decl>(), ::std::forward<Args>(args)...);
				else if (node.is<Expr>())
					visit(node.get<Expr>(), ::std::forward<Args>(args)...);
				else if (node.is<Stmt>())
					visit(node.get<Stmt>(), ::std::forward<Args>(args)...);
				else
					fox_unreachable("Unsupported ASTNode variant");
			}

			// Visit Decl "Dispatch" Method
			DeclRtrTy visit(Decl* decl, Args... args)
			{
				switch (decl->getKind())
				{
					#define DECL(ID,PARENT)\
						case DeclKind::ID:\
							return static_cast<Derived*>(this)->visit##ID(static_cast<ID*>(decl), ::std::forward<Args>(args)...);
					#include "DeclNodes.def"
					default:
						fox_unreachable("Unknown node");
						return DeclRtrTy();
				}
			}

			// Visit Stmt dispatch method
			StmtRtrTy visit(Stmt* stmt, Args... args)
			{
				switch (stmt->getKind())
				{
					#define STMT(ID,PARENT)\
						case StmtKind::ID:\
							return static_cast<Derived*>(this)->visit##ID(static_cast<ID*>(stmt), ::std::forward<Args>(args)...);
					#include "StmtNodes.def"
					default:
						fox_unreachable("Unknown node");
						return StmtRtrTy();
				}
			}

			// Visit Expr dispatch method
			ExprRtrTy visit(Expr* expr, Args... args)
			{
				switch (expr->getKind())
				{
					#define EXPR(ID,PARENT)\
						case ExprKind::ID:\
							return static_cast<Derived*>(this)->visit##ID(static_cast<ID*>(expr), ::std::forward<Args>(args)...);
					#include "ExprNodes.def"
					default:
						fox_unreachable("Unknown node");
						return ExprRtrTy();
				}
			}

			// Visit Types dispatch method
			TypeRtrTy visit(Type* type, Args... args)
			{
				switch (type->getKind())
				{
					#define TYPE(ID,PARENT)\
						case TypeKind::ID:\
							return static_cast<Derived*>(this)->visit##ID(static_cast<ID*>(type), ::std::forward<Args>(args)...);
					#include "TypeNodes.def"
					default:
						fox_unreachable("Unknown node");
						return TypeRtrTy();
				}
			}

			// Base visitStmt, visitDecl and visitType methods.

			// Visit Stmt
			StmtRtrTy visitStmt(Stmt*, Args... args)
			{
				return StmtRtrTy();
			}

			// Visit Expr
			ExprRtrTy visitExpr(Expr*, Args... args)
			{
				return ExprRtrTy();
			}

			// Visit Decl 
			DeclRtrTy visitDecl(Decl*, Args... args)
			{
				return DeclRtrTy();
			}

			// Visit Type 
			TypeRtrTy visitType(Type*, Args... args)
			{
				return TypeRtrTy();
			}

			// VisitXXX Methods
			// The base implementations just chain back to the parent class, so visitors can just
			// implement the parent class or an abstract class and still handle every derived class!
			#define VISIT_METHOD(RTRTYPE, NODE, PARENT)\
			RTRTYPE visit##NODE(NODE* node,Args... args){ \
				return static_cast<Derived*>(this)->visit##PARENT(node, ::std::forward<Args>(args)...); \
			}

			// Decls
			#define DECL(ID,PARENT) VISIT_METHOD(DeclRtrTy, ID, PARENT)
			#define ABSTRACT_DECL(ID,PARENT) VISIT_METHOD(DeclRtrTy, ID, PARENT)
			#include "DeclNodes.def"

			// Stmt & Exprs
			#define STMT(ID,PARENT) VISIT_METHOD(StmtRtrTy, ID, PARENT)
			#define ABSTRACT_STMT(ID,PARENT) VISIT_METHOD(StmtRtrTy, ID, PARENT)
			#include "StmtNodes.def"

			// Stmt & Exprs
			#define EXPR(ID,PARENT) VISIT_METHOD(ExprRtrTy, ID, PARENT)
			#define ABSTRACT_EXPR(ID,PARENT) VISIT_METHOD(ExprRtrTy, ID, PARENT)
			#include "ExprNodes.def"

			// Types
			#define TYPE(ID,PARENT) VISIT_METHOD(TypeRtrTy, ID, PARENT)
			#define ABSTRACT_TYPE(ID,PARENT) VISIT_METHOD(TypeRtrTy, ID, PARENT)
			#include "TypeNodes.def"

	};

	// A Simple visitor where the return type is the same for every node type.
	template<typename Derived, typename RtrTy, typename ... Args>
	using SimpleASTVisitor = ASTVisitor<Derived, RtrTy, RtrTy, RtrTy, RtrTy, Args...>;

}
