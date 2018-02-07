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
// So, this would fail (and that's normal, as variable types can not be changed)
	// x as int = 3

#pragma once
#include "../../IVisitor.h"

namespace Moonshot
{
	class Context;
	class SymbolsTable;
	class RTExprVisitor : public ITypedVisitor<FVal>
	{
		public:
			RTExprVisitor(Context& c,std::shared_ptr<SymbolsTable> symtab);
			RTExprVisitor(Context& c);
			~RTExprVisitor();

			virtual void visit(ASTBinaryExpr & node) override;
			virtual void visit(ASTUnaryExpr & node) override;
			virtual void visit(ASTCastExpr & node) override;

			virtual void visit(ASTLiteral & node) override;
			virtual void visit(ASTVarCall & node) override;

			void setSymbolsTable(std::shared_ptr<SymbolsTable> symtab);
			FVal getResult() const;
		protected:
			// Context
			Context& context_;
			// converts fval to double, but if fval is a varref, deref it first.
			double fvalToDouble_withDeref(FVal fval);
			bool compareVal(const binaryOperation &op, const FVal &l, const FVal &r);
			bool compareStr(const binaryOperation &op, const std::string &lhs, const std::string &rhs);
			double performOp(const binaryOperation& op, double l, double r);
		
			bool fitsInValue(const std::size_t& typ, const double &d); // Determines if we should convert the result to a float when making an operation to avoid loss of information
			
			std::shared_ptr<SymbolsTable> symtab_;
			bool isSymbolsTableAvailable() const;

			// This function calls castTo, but if the FVal is a varRef, it dereferences it once before proceeding.
			FVal castTo_withDeref(const std::size_t& goal, FVal val);
	};

}

