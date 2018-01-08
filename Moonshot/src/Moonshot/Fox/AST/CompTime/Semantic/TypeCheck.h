////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : TypeCheck.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// TypeCheckVisitor visitor.
// This visitor checks for compatibility between operations :
// e.g. can't multiply a string with a int
//
// This visitor also checks that with variables. It gather informations about them (in symtable_)
// when they are declared, and when a variable is used it checks if it was declared
// and if the type is compatible with the current operation.
////------------------------------------------------------////

#pragma once
#include "../../../../Common/Context/Context.h" // context
#include "../../../../Common/Exceptions/Exceptions.h" // exceptions
#include <string> // std::string
#include <sstream> // std::stringstream
#include <map> // std::map
#include <typeinfo> // typeid
#include "../../Visitor/IVisitor.h" // base class
#include "../../../../Common/Types/Types.h" // FVal Utilities
#include "../../../../Common/Symbols/Symbols.h" // symbols table

// Include nodes
#include "../../Nodes/ASTExpr.h" 
#include "../../Nodes/ASTVarDeclStmt.h" 
#include "../../../Util/Enums.h" // enums

namespace Moonshot
{
	class TypeCheckVisitor : public IVisitor
	{
		public:
			TypeCheckVisitor(Context& c,const bool& testmode = false);
			~TypeCheckVisitor();

			virtual void visit(ASTExpr & node) override;
			virtual void visit(ASTRawValue & node) override;

			virtual void visit(ASTVarDeclStmt & node) override;
			virtual void visit(ASTVarCall & node) override;

			SymbolsTable symtable_; // The symbols table used to track variable declarations and types.
			// it is public so we can add anything we want to it for testing purposes.
		private:
			// Context
			Context& context_;
			template<typename T>
			inline std::size_t visitAndGetResult(std::unique_ptr<T>& node,const dir& dir = dir::UNKNOWNDIR)
			{
				// curdir is a variable that's there to help some function behave. Like
				// the ASTVarcall function overload that can use this to check if the variable is the subject of an assignement!
				curdir_ = dir;
				node->accept(*this);
				curdir_ = dir::UNKNOWNDIR; 
				return rtr_type_;
			}
			operation curop_;
			dir curdir_;
			std::size_t rtr_type_; // Last returned type from visiting node (held here, because visit doesn't return anything :( )
			std::size_t getExprResultType(const operation& op, std::size_t& lhs, const std::size_t& rhs);

	};

}

