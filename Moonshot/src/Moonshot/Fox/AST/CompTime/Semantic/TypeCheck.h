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
	class TypeCheckVisitor : public ITypedVisitor<std::size_t> // size_t because we return indexes in FVal to represent types.
	{
		public:
			TypeCheckVisitor(Context& c,const bool& testmode = false);
			~TypeCheckVisitor();

			virtual void visit(ASTExpr & node) override;
			virtual void visit(ASTLiteral & node) override;

			virtual void visit(ASTVarDeclStmt & node) override;
			virtual void visit(ASTVarCall & node) override;

			SymbolsTable symtable_; // The symbols table used to track variable declarations and types.
			// it is public so we can add anything we want to it for testing purposes.
		private:
			// Context
			Context& context_;

			// Utiliser un template aussi pour *this?
			template<typename T>
			inline std::size_t visitAndGetResult(std::unique_ptr<T>& node,const dir& dir = dir::UNKNOWNDIR)
			{
				node_ctxt_.dir = dir;
				node->accept(*this);
				node_ctxt_.dir = dir::UNKNOWNDIR;
				return value_;
			}

			bool shouldOpReturnFloat(const operation& op) const; // used for operations that return float instead of normal values
			std::size_t getExprResultType(const operation& op, std::size_t& lhs, const std::size_t& rhs);

			struct nodecontext
			{
				dir dir;
				operation op;
			} node_ctxt_;
	};

}

