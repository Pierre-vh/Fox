////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : RTExprVisitor.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file evaluates Expression trees and retursna  FoxValue with the result.
// It's simple ! At first it was used for debugging, but this class might
// be used in a more definitive version of the project because it could
// prove itself useful when doing "Constant Folding". 
// I Just need to check whether the tree is a constant expression 
// (Only const variable calls and raw values)
////------------------------------------------------------////

#pragma once
#include "Moonshot/Fox/AST/IVisitor.hpp"
#include <memory>
namespace Moonshot
{
	class Context;
	class DataMap;
	class FoxType;
	enum class binaryOperator;
	class RTExprVisitor : public ITypedVisitor<FoxValue>
	{
		public:
			RTExprVisitor(Context& c,std::shared_ptr<DataMap> symtab);
			RTExprVisitor(Context& c);
			~RTExprVisitor();

			virtual void visit(ASTBinaryExpr & node) override;
			virtual void visit(ASTUnaryExpr & node) override;
			virtual void visit(ASTCastExpr & node) override;

			virtual void visit(ASTLiteralExpr & node) override;
			virtual void visit(ASTVarRefExpr & node) override;

			void setDataMap(std::shared_ptr<DataMap> symtab);
			FoxValue getResult() const;
		protected:
			// Context
			Context& context_;
			// converts fval to double, but if fval is a varref, deref it first.
			double fvalToDouble(FoxValue fval);
			bool compareVal(const binaryOperator &op, const FoxValue &l, const FoxValue &r);
			bool compareStr(const binaryOperator &op, const std::string &lhs, const std::string &rhs);
			bool compareChar(const binaryOperator &op, const CharType& lhs, const CharType& rhs);
			FoxValue concat(const FoxValue& lhs, const FoxValue& rhs);
			double performOp(const binaryOperator& op, double l, double r);
		
			bool fitsInValue(const FoxType& typ, const double &d); // Determines if we should convert the result to a float when making an operation to avoid loss of information
			
			std::shared_ptr<DataMap> datamap_;
			bool isDataMapAvailable() const;

			void deref(FoxValue& val) const;
	};

}

