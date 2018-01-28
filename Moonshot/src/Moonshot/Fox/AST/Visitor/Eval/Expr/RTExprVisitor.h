////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : RTExprVisitor.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file evaluates Expression trees and retursna  FVal with the result.
// It's simple ! At first it was used for debugging, but this class might
// be used in a more definitive version of the project because it could
// prove itself useful when doing "Constant Folding". 
// I Just need to check whether the tree is a constant expression 
// (Only const variable calls and raw values)
////------------------------------------------------------////

	// Notes about behaviour //
// When evaluating variables calls, this class returns a reference to the variable.
// This class will attempt to deref reference first at some points. When cast, it returns the value of the variable
// and not a reference to it! 
// So, in theory, this would fail (and that's normal, as variable types can not be changed)
	// x as int = 3

#pragma once
// Context & exceptions
#include "../../../../../Common/Context/Context.h"
#include "../../../../../Common/Exceptions/Exceptions.h"
// Utils, types, typecast
#include "../../../../../Common/Utils/Utils.h"
#include "../../../../../Common/Types/Types.h"
#include "../../../../../Common/Types/TypeCast.h"
// Symbols table
#include "../../../../../Common/Symbols/Symbols.h"
#include "../../../Nodes/ASTExpr.h"
#include "../../../Nodes/ASTVarDeclStmt.h"
#include "../../IVisitor.h"

#include <tuple>		// Pair
#include <climits>		// Variable limits
#include <cmath>		// C++ math operations
#include <variant>		// holds_alternative


namespace Moonshot
{
	class RTExprVisitor : public IVisitor
	{
		public:
			RTExprVisitor(Context& c,std::shared_ptr<SymbolsTable> symtab);
			RTExprVisitor(Context& c);
			~RTExprVisitor();

			virtual void visit(ASTExpr & node) override;
			virtual void visit(ASTLiteral & node) override;
			virtual void visit(ASTVarCall & node) override;

			void setSymbolsTable(std::shared_ptr<SymbolsTable> symtab);
			FVal getResult() const;
		protected:
			FVal value_;
			// Context
			Context& context_;
			// converts fval to double, but if fval is a varref, deref it first.
			double fvalToDouble_withDeref(FVal fval);
			bool compareVal(const operation &op, const FVal &l, const FVal &r);
			bool compareStr(const operation &op, const std::string &lhs, const std::string &rhs);
			double performOp(const operation& op, double l, double r);
		
			bool fitsInValue(const std::size_t& typ, const double &d); // Determines if we should convert the result to a float when making an operation to avoid loss of information
			
			std::shared_ptr<SymbolsTable> symtab_;
			bool isSymbolsTableAvailable() const;

			// This function calls castTo, but if the FVal is a varRef, it dereferences it once before proceeding.
			FVal castTo_withDeref(const std::size_t& goal, FVal val);

			//Visit & get result
			template<typename VISITABLE>
			FVal visitAndGetResult(VISITABLE& node)
			{
				node->accept(*this);
				return value_;
			}
	};

}

