#pragma once
#include <string> // std::string
#include <type_traits> // std::is_same
#include <sstream> // std::stringstream
#include <variant> // std::visit
#include <typeinfo> // typeid
#include "../IVisitor.h" // base class
#include "../../../../Common/Types/Types.h" // FVal Utilities

// Include nodes
#include "../../Nodes/ASTExpr.h" 
#include "../../Nodes/ASTVarDeclStmt.h" 

#include "../../../Util/Enums.h" // enums


namespace Moonshot
{
	class TypeCheck : public IVisitor
	{
		public:
			TypeCheck();
			~TypeCheck();

			virtual void visit(ASTExpr * node) override;
			virtual void visit(ASTValue * node) override;


			virtual void visit(ASTVarDeclStmt * node) override;

			std::size_t getReturnTypeOfExpr() const;
		private:
			std::size_t rtr_type_; // Last returned type from visiting node (held here, because visit doesn't return anything :( )
			std::size_t getExprResultType(const parse::optype& op, std::size_t& lhs, const std::size_t& rhs);

	};

}

