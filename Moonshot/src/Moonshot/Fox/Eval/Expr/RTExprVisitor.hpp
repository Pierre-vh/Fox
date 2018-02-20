////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : RTExprVisitor.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file evaluates Expression trees and retursna  FVal with the result.
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
	enum class binaryOperator;
	class RTExprVisitor : public ITypedVisitor<FVal>
	{
		public:
			RTExprVisitor(Context& c,std::shared_ptr<DataMap> symtab);
			RTExprVisitor(Context& c);
			~RTExprVisitor();

			virtual void visit(ASTBinaryExpr & node) override;
			virtual void visit(ASTUnaryExpr & node) override;
			virtual void visit(ASTCastExpr & node) override;

			virtual void visit(ASTLiteral & node) override;
			virtual void visit(ASTVarCall & node) override;

			void setDataMap(std::shared_ptr<DataMap> symtab);
			FVal getResult() const;
		protected:
			// Context
			Context& context_;
			// converts fval to double, but if fval is a varref, deref it first.
			double fvalToDouble_withDeref(FVal fval);
			bool compareVal(const binaryOperator &op, const FVal &l, const FVal &r);
			bool compareStr(const binaryOperator &op, const std::string &lhs, const std::string &rhs);
			bool compareChar(const binaryOperator &op, const CharType& lhs, const CharType& rhs);
			FVal concat(const FVal& lhs, const FVal& rhs);
			double performOp(const binaryOperator& op, double l, double r);
		
			bool fitsInValue(const std::size_t& typ, const double &d); // Determines if we should convert the result to a float when making an operation to avoid loss of information
			
			std::shared_ptr<DataMap> symtab_;
			bool isDataMapAvailable() const;

			// This function calls castTo, but if the FVal is a varRef, it dereferences it once before proceeding.
			FVal castTo_withDeref(const std::size_t& goal, FVal val);
	};

}

