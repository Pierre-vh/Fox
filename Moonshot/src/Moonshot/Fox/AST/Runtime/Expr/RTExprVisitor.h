#pragma once
//utils
#include "../../../../Common/Utils/Utils.h"
#include "../../../../Common/Errors/Errors.h"
#include "../../../../Common/FValue/FValue.h"

#include "../../Nodes/ASTExpr.h"
#include "../../Nodes/ASTVarDeclStmt.h"

#include "../IRTVisitor.h"
#include <tuple>		// Pair
#include <climits>		// Variable limits
#include <cmath>		// C++ math operations
#include <variant>		// holds_alternative

// Necessary?
//#include <variant> // std::bad_variant_access

namespace Moonshot
{
	class RTExprVisitor : public IRTVisitor
	{
		public:
			RTExprVisitor();
			~RTExprVisitor();

			// Inherited via IRTVisitor
			virtual FVal visit(ASTExpr * node) override;
			virtual FVal visit(ASTValue * node) override;
		private:
			double fvalToDouble(const FVal &fval);
			bool compareVal(const parse::optype &op, const FVal &l, const FVal &r);
			bool compareStr(const parse::optype &op, const std::string &lhs, const std::string &rhs);
			double performOp(const parse::optype& op, const double &l, const double &r);
		
			bool fitsInValue(const std::size_t& typ, const double &d); // Determines if we should convert the result to a float when making an operation to avoid loss of information
			class castHelper
			{
				public:
					FVal castTo(const std::size_t& goal, const FVal& val);
					FVal castTo(const std::size_t& goal, const double &val);
				private:
					template<typename GOAL,typename VAL, bool b1 = std::is_same<GOAL,std::string>::value,bool b2 = std::is_same<VAL,std::string>::value>
					std::pair<bool, FVal> castTypeTo(const GOAL& type,VAL v); 

					template<typename GOAL>
					std::pair<bool, FVal> castTypeTo(const GOAL& type,double v);
					
			};
	};

}

