#pragma once

#include "IASTStmt.h"
#include "ASTExpr.h"

#include <string> // std::string
#include <memory> // std::unique_ptr, std::make_unique

namespace Moonshot 
{
	namespace var
	{
		struct varattr // Struct holding a var's attributes
		{
			varattr();
			varattr(const std::string &nm, const std::size_t &ty, const bool &isK = false);
			operator bool() const;
			// Variable's attribute
			bool isConst = false;
			std::string name = "";
			std::size_t type = fval_void;

			private:
				bool wasInit_ = false;
		};
	}
	struct ASTVarDeclStmt : public IASTStmt
	{
		public:
			// Create a variable declaration statement by giving the constructor the variable's properties (name,is const and type) and, if there's one, an expression to initialize it.
			ASTVarDeclStmt(const var::varattr &attr,std::unique_ptr<ASTExpr>& iExpr = std::make_unique<ASTExpr>(nullptr)); 
			~ASTVarDeclStmt();

			// Inherited via IASTStmt
			virtual void accept(IVisitor& vis) override;
			virtual FVal accept(IRTVisitor& vis) override;

			var::varattr vattr_;
			std::unique_ptr<ASTExpr> initExpr_;
	};
}


