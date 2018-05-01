////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTVisitor.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file implements the "Visitor" class, which dispatchs Stmt,Decls and Types
// to their appropriate "visit" method.
//
// This visitor is heavily based on Swift's : https://github.com/apple/swift/blob/master/include/swift/AST/ASTVisitor.h
////------------------------------------------------------////

#pragma once

#include "Decl.hpp"
#include "Stmt.hpp"
#include "Expr.hpp"
#include "Type.hpp"

#include <exception>
#include <utility>

namespace Moonshot
{
	// Visitor class, which takes a few templates arguments : The derived class, The return type and the Args
	// that should be passed to the Visit method.
	template<typename Derived,typename ReturnType,typename ... Args>
	class ASTVisitor
	{
		public:
			// Visit Decl "Dispatch" Method
			ReturnType visit(Decl *decl, Args... VArgs)
			{
				switch (decl->getKind())
				{
					#define DECL(ID,PARENT)\
						case DeclKind::ID:\
							return static_cast<Derived*>(this)->visit##ID(static_cast<ID*>(decl),std::forward<Args>(VArgs)...);
					#include "DeclNodes.def"
					default:
						throw std::exception("Unreachable.");
				}
			}

			// Visit Stmt (& Expr) dispatch method
			ReturnType visit(Stmt *stmt, Args... VArgs)
			{
				switch (stmt->getKind())
				{
					#define STMT(ID,PARENT)\
						case StmtKind::ID:\
							return static_cast<Derived*>(this)->visit##ID(static_cast<ID*>(stmt),std::forward<Args>(VArgs)...);
					#include "StmtNodes.def"
					default:
						throw std::exception("Unreachable.");
				}
			}

			// Visit Types dispatch method
			ReturnType visit(Type *type, Args... VArgs)
			{
				switch (type->getKind())
				{
					#define TYPE(ID,PARENT)\
						case TypeKind::ID:\
							return static_cast<Derived*>(this)->visit##ID(static_cast<ID*>(type),std::forward<Args>(VArgs)...);
					#include "TypeNodes.def"
					default:
						throw std::exception("Unreachable.");
				}
			}

			// Base visitStmt, visitDecl and visitType methods.

			// Visit Stmt (works on Exprs too)
			ReturnType visitStmt(Stmt* node, Args... VArgs)
			{
				return ReturnType();
			}

			// Visit Decl 
			ReturnType visitDecl(Decl* node, Args... VArgs)
			{
				return ReturnType();
			}

			// Visit Type 
			ReturnType visitType(Type* node, Args... VArgs)
			{
				return ReturnType();
			}

			// VisitXXX Methods
			// The base implementations just chain back to the parent class, so visitors can just
			// implement the parent class or an abstract class and still handle every derived class!
			#define VISIT_METHOD(NODE,PARENT)\
			ReturnType visit##NODE(NODE* node,Args... VArgs){ \
				return static_cast<Derived*>(this)->visit##PARENT(node,std::forward<Args>(VArgs)...); \
			}

			// Decls
			#define DECL(ID,PARENT) VISIT_METHOD(ID,PARENT)
			#define ABSTRACT_DECL(ID,PARENT) VISIT_METHOD(ID,PARENT)
			#include "DeclNodes.def"

			// Stmt & Exprs
			#define STMT(ID,PARENT) VISIT_METHOD(ID,PARENT)
			#define ABSTRACT_STMT(ID,PARENT) VISIT_METHOD(ID,PARENT)
			#include "StmtNodes.def"

			// Types
			#define TYPE(ID,PARENT) VISIT_METHOD(ID,PARENT)
			#define ABSTRACT_TYPE(ID,PARENT) VISIT_METHOD(ID,PARENT)
			#include "TypeNodes.def"

	};
}

#undef DERIVED_PTR