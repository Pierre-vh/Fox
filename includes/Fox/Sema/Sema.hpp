////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Sema.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Contains the Sema class, which is used to perform 
// most of the semantic analysis of a Fox AST.
// 
// To-Do: Find a better name than "checkExpr". It needs to be
// clear that it should only be used as the entry point for checking any
// expression!
//
//	Find another name for Result too, since it could be confusing with Parser::Result?
////------------------------------------------------------//// 

#include <cstdint>
#include "Fox/AST/ASTNode.hpp"
#include "Fox/AST/ASTFwdDecl.hpp"
#include "Fox/Common/ResultObject.hpp"

namespace fox
{
	class Sema
	{
		public:
			// Typedefs
			using IntegralRankTy = std::uint8_t;

			template<typename NodeTy>
			class Result : public ResultObject<NodeTy>
			{
				using Inherited = ResultObject<NodeTy>;
				using ThisTy = Result<NodeTy>;
				public:
					using Inherited::ResultObject;

					static ThisTy Success(CTorValueTy val = DefaultValue())
					{
						return Result(true, val);
					}

					static ThisTy Failure()
					{
						return Result(false, DefaultValue());
					}

					const NodeTy getReplacement() const
					{
						return get();
					}

					NodeTy getReplacement()
					{
						return get();
					}

					bool hasReplacement() const
					{
						return hasData();
					}
					
				private:
					NodeTy* node_ = nullptr;
					bool success_ : 1;
					// 7 Bits left in bitfield
			};

			using ExprResult = Result<Expr*>;

			// Entry point for checking an expression tree.
			ExprResult checkExpr(Expr* expr);

			// The unification algorithms for types of the same subtypes.
			//
			// SemaType with no subs. + Any type -> True
			// Any Type & Any Type -> returns compareSubtype(a,b)
			// False in all other cases.
			static bool unifySubtype(Type* a, Type* b);

			// Compares the Subtypes of A and B. Returns true if a and b
			// share the same "subtype", false otherwise.
			static bool compareSubtypes(Type* a, Type* b);

			// This method returns the integral rank that a given type has.
			// type must not be null and must point to a arithmetic type.
			static IntegralRankTy getIntegralRank(PrimitiveType* type);

		private:
			class ExprChecker;

			ExprResult checkParensExpr(ParensExpr* node);
			ExprResult checkBinaryExpr(BinaryExpr* node);
			ExprResult checkUnaryExpr(UnaryExpr* node);
			ExprResult checkCastExpr(CastExpr* node);
			ExprResult checkArrayAccessExpr(ArrayAccessExpr* node);
			ExprResult checkCharLiteralExpr(CharLiteralExpr* node);
			ExprResult checkBoolLiteralExpr(BoolLiteralExpr*node);
			ExprResult checkIntegerLiteralExpr(IntegerLiteralExpr* node);
			ExprResult checkFloatLiteralExpr(FloatLiteralExpr* node);
			ExprResult checkStringLiteralExpr(StringLiteralExpr* node);
			ExprResult checkArrayLiteralExpr(ArrayLiteralExpr* node);
			ExprResult checkDeclRefExpr(DeclRefExpr* node);
			ExprResult checkMemberOfExpr(MemberOfExpr* node);
			ExprResult checkFunctionCallExpr(FunctionCallExpr* node);
	};
}