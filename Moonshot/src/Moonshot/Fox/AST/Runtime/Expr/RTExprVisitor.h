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

#pragma once
#include "../../../../Common/Utils/Utils.h"
#include "../../../../Common/Errors/Errors.h"
#include "../../../../Common/Types/Types.h"
#include "../../../../Common/Symbols/Symbols.h"

#include "../../Nodes/ASTExpr.h"
#include "../../Nodes/ASTVarDeclStmt.h"

#include "../IRTVisitor.h"
#include <tuple>		// Pair
#include <climits>		// Variable limits
#include <cmath>		// C++ math operations
#include <variant>		// holds_alternative


namespace Moonshot
{
	class RTExprVisitor : public IRTVisitor
	{
		public:
			RTExprVisitor(std::shared_ptr<SymbolsTable> symtab);
			RTExprVisitor();
			~RTExprVisitor();

			virtual FVal visit(ASTExpr & node) override;
			virtual FVal visit(ASTRawValue & node) override;
			virtual FVal visit(ASTVarCall & node) override;

			void setSymbolsTable(std::shared_ptr<SymbolsTable> symtab);

		protected:
			double fvalToDouble(const FVal &fval);
			bool compareVal(const parse::optype &op, const FVal &l, const FVal &r);
			bool compareStr(const parse::optype &op, const std::string &lhs, const std::string &rhs);
			double performOp(const parse::optype& op, double l, double r);
		
			bool fitsInValue(const std::size_t& typ, const double &d); // Determines if we should convert the result to a float when making an operation to avoid loss of information
			
			std::shared_ptr<SymbolsTable> symtab_;
			bool isSymbolsTableAvailable() const;
		private:
			FVal castTo(const std::size_t& goal, FVal val);
			FVal castTo(const std::size_t& goal, const double &val);

			template<typename GOAL,typename VAL, bool isGOALstr = std::is_same<GOAL,std::string>::value,bool isVALstr = std::is_same<VAL,std::string>::value>
			std::pair<bool, FVal> castTypeTo(const GOAL& type,VAL v); 

			template<typename GOAL>
			std::pair<bool, FVal> castTypeTo(const GOAL& type,double v);
	};

}

