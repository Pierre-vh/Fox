////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTWalker.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file contains the ASTWalker class, which is used to
// "walk" the ast in a pre/post order fashion, automatically 
// replacing nodes if needed.
//
////------------------------------------------------------////

#include <tuple>
#include "Fox/AST/ASTFwdDecl.hpp"

namespace fox
{
	class ASTWalker
	{
		public:
			ASTNode walk(ASTNode node);
			Expr* walk(Expr* expr);
			Decl* walk(Decl* decl);
			Stmt* walk(Stmt* stmt);
			Type* walk(Type* type);

			virtual std::pair<bool, Expr*> handleExprPre(Expr* expr);
			virtual Expr* handleExprPost(Expr* expr);

			virtual std::pair<bool, Stmt*> handleStmtPre(Stmt* stmt);
			virtual Stmt* handleStmtPost(Stmt* stmt);

			virtual std::pair<bool, Decl*> handleDeclPre(Decl* decl);
			virtual Decl* handleDeclPost(Decl* decl);

			virtual std::pair<bool, Type*> handleTypePre(Type* type);
			virtual Type* handleTypePost(Type* type);
	};
}