#pragma once
#include <string> // std::string
#include <type_traits> // std::is_same
#include <sstream> // std::stringstream
#include <variant> // std::visit
#include <map>
#include <typeinfo> // typeid
#include "../../Visitor/IVisitor.h" // base class
#include "../../../../Common/Types/Types.h" // FVal Utilities
#include "../../../../Common/Symbols/Symbols.h"
// Include nodes
#include "../../Nodes/ASTExpr.h" 
#include "../../Nodes/ASTVarDeclStmt.h" 
#include "../../../Util/Enums.h" // enums
// Todo : add support for variable:
// Assignement : check if type of lhs is assignable to the type of the rhs.
// Declaration : check if the type of the variable declared is assignable to the type of the initexpr.
// Var use : return the var type.
namespace Moonshot
{
	class TypeCheck : public IVisitor
	{
		public:
			TypeCheck(const bool& testmode = false);
			~TypeCheck();

			virtual void visit(ASTExpr & node) override;
			virtual void visit(ASTRawValue & node) override;

			virtual void visit(ASTVarDeclStmt & node) override;
			virtual void visit(ASTVarCall & node) override;

			SymbolsTable symtable_; // The symbols table used to track variable declarations and types.
			// it is public so we can add anything we want to it for testing purposes.
		private:
			template<typename T>
			inline std::size_t visitAndGetResult(std::unique_ptr<T>& node,const parse::direction& dir = parse::direction::UNKNOWNDIR)
			{
				// curdir is a variable that's there to help some function behave. Like
				// the ASTVarcall function overload that can use this to check if the variable is the subject of an assignement!
				curdir_ = dir;
				node->accept(*this);
				curdir_ = parse::direction::UNKNOWNDIR; 
				return rtr_type_;
			}
			parse::optype curop_;
			parse::direction curdir_;
			std::size_t rtr_type_; // Last returned type from visiting node (held here, because visit doesn't return anything :( )
			std::size_t getExprResultType(const parse::optype& op, std::size_t& lhs, const std::size_t& rhs);

	};

}

