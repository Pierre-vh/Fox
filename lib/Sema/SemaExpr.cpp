//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : SemaExpr.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  This file implements Sema methods related to Exprs and most of the 
//  expr checking logic.
//----------------------------------------------------------------------------//

#include "Fox/Sema/Sema.hpp"
#include "Fox/AST/Expr.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/Common/LLVM.hpp"
#include "Fox/AST/ASTWalker.hpp"
#include "Fox/AST/ASTVisitor.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"

#include <utility>
#include <iostream>

using namespace fox;

namespace {
  // Expression checker: Classic visitor, the visitXXX functions
  // all check a single node. They do not orchestrate visitation of
  // the children, because that is done in the ASTWalker
  //
  // Every visitation method return a pointer to an Expr*, which is the current 
  // expr
  // OR the expr that should take it's place. This can NEVER be null.
  class ExprChecker : ExprVisitor<ExprChecker, Expr*>,  ASTWalker {
    using Inherited = ExprVisitor<ExprChecker, Expr*>;
    Sema& sema_;
    friend class Inherited;
    public:
      ExprChecker(Sema& sema): sema_(sema) {
        
      }

      // Returns the ASTContext
      ASTContext& getCtxt() {
        return sema_.getASTContext();
      }

      // Returns the DiagnosticEngine
      DiagnosticEngine& getDiags() {
        return sema_.getDiagnosticEngine();
      }

      Sema& getSema() {
        return sema_;
      }

      // Entry point
      Expr* check(Expr* expr) {
        return walk(expr);
      }
    private:
      //----------------------------------------------------------------------//
      // Diagnostic methods
      //----------------------------------------------------------------------//
      // The diagnose family of methods are designed to print the most relevant
      // diagnostics for a given situation.
      //----------------------------------------------------------------------//

      // (Error) Diagnoses an invalid cast 
      void diagnoseInvalidCast(CastExpr* expr) {
        SourceRange range = expr->getCastTypeLoc().getRange();
        Type childTy = expr->getExpr()->getType();
        Type goalTy = expr->getCastTypeLoc().withoutLoc();
        getDiags()
          .report(DiagID::sema_invalid_cast, range)
          .addArg(childTy)
          .addArg(goalTy)
          .setExtraRange(expr->getExpr()->getRange());
      }

      // (Warning) Diagnoses a redudant cast (when the
      // cast goal and the child's type are equal)
      void diagnoseRedundantCast(CastExpr* expr) {
        SourceRange range = expr->getCastTypeLoc().getRange();
        Type goalTy = expr->getCastTypeLoc().withoutLoc();
        getDiags()
          .report(DiagID::sema_redundant_cast, range)
          .addArg(goalTy)
          .setExtraRange(expr->getExpr()->getRange());
      }

      void diagnoseHeteroArrLiteral(ArrayLiteralExpr* expr, Expr* faultyElem) {
        if (faultyElem) {
          getDiags()
            // Precise error loc is the first element that failed the inferrence,
            // extended range is the whole arrayliteral's.
            .report(DiagID::sema_arraylit_hetero, faultyElem->getRange())
            .setRange(expr->getRange());
        }
        else {
          getDiags()
            // If we have no element to pinpoint, just use the whole expr's
            // range
            .report(DiagID::sema_arraylit_hetero, expr->getRange());
        }
      }

      void diagnoseInvalidUnaryOpChildType(UnaryExpr* expr) {
        Expr* child = expr->getExpr();
        Type childTy = child->getType();
        getDiags()
          .report(DiagID::sema_unaryop_bad_child_type, expr->getOpRange())
          // Use the child's range as the extra range.
          .setExtraRange(child->getRange())
          .addArg(expr->getOpSign()) // %0 is the operator's sign as text
          .addArg(childTy); // %1 is the type of the child
      }

      void diagnoseInvalidArraySubscript(ArraySubscriptExpr* expr,
                                         SourceRange range, 
                                         SourceRange extra) {
        Expr* child = expr->getBase();
        Type childTy = child->getType();

        Expr* idxE = expr->getIndex();
        Type idxETy = idxE->getType();

        getDiags()
          .report(DiagID::sema_arrsub_invalid_types, range)
          // %0 is subscripted value's type, %1 is the index's expr type;
          .addArg(childTy)
          .addArg(idxETy)
          .setExtraRange(extra);
      }

      void diagnoseInvalidBinaryExprOperands(BinaryExpr* expr) {
        SourceRange opRange = expr->getOpRange();
        SourceRange exprRange = expr->getRange();
        Type lhsTy = expr->getLHS()->getType();
        Type rhsTy = expr->getRHS()->getType();
        getDiags()
          .report(DiagID::sema_binexpr_invalid_operands, opRange)
          .addArg(expr->getOpSign())
          .addArg(lhsTy)
          .addArg(rhsTy);
      }

      // Warns about an implicit integral downcast from float to int
      void warnImplicitIntegralDowncast(Type exprTy, Type destTy,
                                        SourceRange range,
                                        SourceRange extra = SourceRange()) {
        auto diag = 
          getDiags()
            .report(DiagID::sema_implicit_integral_downcast, range)
            .addArg(exprTy)
            .addArg(destTy);

        if (extra)
          diag.setExtraRange(extra);
      }

      //----------------------------------------------------------------------//
      // Finalize methods
      //----------------------------------------------------------------------//
      // The finalize family of methods will perform the finalization of a 
      // given expr. Example cases where a finalize method should be created:
      //    -> The finalization logic doesn't fit in 2 or 3 lines of code
      //    -> The finalization logic is repetitive and called multiple times
      //    -> Improving readability
      //
      // Theses methods will often just set the type of the expr, maybe emit
      // some warnings too.
      //
      // /!\ ALL FINALIZE METHODS ASSUME THAT THE EXPR IS SEMANTICALLY CORRECT!
      //----------------------------------------------------------------------//

      // Finalizes an expression whose type is boolean (e.g. conditional/logical
      // expressions such as LAnd, LNot, LE, GT, etc..)
      Expr* finalizeBooleanExpr(Expr* expr) {
        expr->setType(PrimitiveType::getBool(getCtxt()));
        return expr;
      }

      // Finalizes a CastExpr
      Expr* finalizeCastExpr(CastExpr* expr, bool isRedundant) {
        if (isRedundant) {
          // Diagnose the redundant cast (emit a warning).
          diagnoseRedundantCast(expr);
          // Remove the CastExpr and just return the child
          Expr* child = expr->getExpr();
          // Simply replace the range of the child with the range
          // of the CastExpr, so diagnostics will correctly highlight the whole
          // cast's region.
          child->setRange(expr->getRange());
          return child;
        }

        // Else, the Expr's type is simply the castgoal.
        expr->setType(expr->getCastTypeLoc().withoutLoc());
        return expr;
      }

      // Finalizes a UnaryExpr
      // \param childTy The type of the child as a PrimitiveType.
      Expr* finalizeUnaryExpr(UnaryExpr* expr, PrimitiveType* childTy) {
        assert(childTy && "cannot be nullptr");
        using OP = UnaryExpr::OpKind;
        switch (expr->getOp()) {
          // Logical NOT operator : '!'
          case OP::LNot:
            return finalizeBooleanExpr(expr);
          // Unary Plus '+' and Minus '-'
          case OP::Minus:
          case OP::Plus:
            // Always int or float, never bool, so uprank
            // if boolean.
            expr->setType(uprankIfBoolean(childTy));
            return expr;
          case OP::Invalid:
            fox_unreachable("Invalid Unary Operator");
          default:
            fox_unreachable("All cases handled");
        }
      }

      // Finalizes an empty Array Literal
      Expr* finalizeEmptyArrayLiteral(ArrayLiteralExpr* expr) {
        assert((expr->getSize() == 0) && "Only for empty Array Literals");
        // For empty array literals, the type is going to be a fresh
        // celltype inside an Array : Array(CellType(null))
        Type type = CellType::create(getCtxt());
        type = ArrayType::get(getCtxt(), type); 
        expr->setType(type);
        return expr;
      }

      // Finalizes an Array Subscript Expr 
      Expr* finalizeArraySubscriptExpr(ArraySubscriptExpr* expr, 
                                       Type childTy) {    
        Type exprTy = childTy->unwrapIfArray();
        assert(exprTy && 
               "Expression is valid but childTy is not an ArrayType?");
        expr->setType(exprTy);
        return expr;
      }

      // Finalizes a valid concatenation binary operation.
      Expr* finalizeConcatBinaryExpr(BinaryExpr* expr) {
        // For concatenation, the type is always string.
        // We'll also change the add operator to become the concat operator.
        expr->setType(PrimitiveType::getString(getCtxt()));
        expr->setOp(BinaryExpr::OpKind::Concat);
        return expr;
      }

      //----------------------------------------------------------------------//
      // ASTWalker overrides
      //----------------------------------------------------------------------//

      virtual std::pair<Expr*, bool> handleExprPre(Expr* expr) {
        // Not needed since we won't do preorder visitation
        return { expr, true }; // Important for postorder visitation to be done
      }

      virtual Expr* handleExprPost(Expr* expr) {
        assert(expr && "Expr cannot be null!");
        expr = visit(expr);
        assert(expr && "Expr cannot be null!");
        // Check if the expr is typed. If it isn't, that
        // means typechecking failed for this node, so set
        // it's type to ErrorType.
        if (!expr->getType()) {
          expr->setType(ErrorType::get(getCtxt()));
        }
        return expr;
      }

      virtual std::pair<Stmt*, bool> handleStmtPre(Stmt*) {
        fox_unreachable("Illegal node kind");
      }

      virtual std::pair<Decl*, bool> handleDeclPre(Decl*) {
        fox_unreachable("Illegal node kind");
      }

      //----------------------------------------------------------------------//
      // "visit" methods
      //----------------------------------------------------------------------//
      // Theses visit() methods will perform the necessary tasks to check a
      // given expr. In trivial/simple cases, theses methods will do everything
      // needed to typecheck the expr (emit diagnostics, finalize, etc), but
      // they may also delegate the work to some check/finalize methods.
      //----------------------------------------------------------------------//

      Expr* visitBinaryExpr(BinaryExpr* expr) {
        using BOp = BinaryExpr::OpKind;

        assert(expr->isValidOp() &&
          "Operation is invalid");

        // Fetch the types of the LHS and RHS 
        Type lhsTy = expr->getLHS()->getType();
        Type rhsTy = expr->getRHS()->getType();

        // Check that they aren't ErrorTypes. If they are, don't bother
        // checking.
        if (lhsTy->is<ErrorType>() || rhsTy->is<ErrorType>())
          return expr;

        // Handle assignements early, let checkAssignementBinaryExpr do it.
        if (expr->isAssignement())
          return checkAssignementBinaryExpr(expr, lhsTy, rhsTy);

        // For every other operator, we must use the bound RValue version
        // of the types.
        lhsTy = lhsTy->getAsBoundRValue();
        rhsTy = rhsTy->getAsBoundRValue();

        // If the types are not bound, just give up and let the ExprFinalizer
        // display the errors.
        if (!(lhsTy && rhsTy)) return expr;

        switch (BOp op = expr->getOp()) {
          // Multiplicative, additive and exponent binary expr
          // are checked by checkBasicIntegralBinaryExpr, except
          // concatenations which are directly finalized through
          // finalizeConcatBinaryExpr
          case BOp::Add:
            if (canConcat(op, lhsTy, rhsTy))
              return finalizeConcatBinaryExpr(expr);
            // (else) fall through 
          case BOp::Sub:
          case BOp::Mul:
          case BOp::Div:
          case BOp::Mod:
          case BOp::Exp:
            return checkBasicIntegralBinaryExpr(expr, lhsTy, rhsTy);
          // Assignements
          case BOp::Assign:
            return checkAssignementBinaryExpr(expr, lhsTy, rhsTy);
          // Comparisons
          case BOp::Eq:
          case BOp::NEq:
          case BOp::GE:
          case BOp::GT:
          case BOp::LE:
          case BOp::LT:
            return checkComparisonBinaryExpr(expr, lhsTy, rhsTy);
          // Logical operators
          case BOp::LAnd:
          case BOp::LOr:
            return checkLogicalBinaryExpr(expr, lhsTy, rhsTy);
          default:
            fox_unreachable("All cases handled");
        }
      }

      Expr* visitCastExpr(CastExpr* expr) {        
        // Get the types & unwrap them
        Type childTy = expr->getExpr()->getType();
        Type goalTy = expr->getCastTypeLoc().withoutLoc();
        std::tie(childTy, goalTy) = Sema::unwrapAll({childTy, goalTy });

        // Sanity Check:
          // It is impossible for unbound types to exist
          // as cast goals, as cast goals are type written
          // down by the user.
        assert(goalTy->isBound() &&
          "Unbound types cannot be present as cast goals!");

        // Check for Error Types. If one of the types is an ErrorType
        // just abort.
        if (childTy->is<ErrorType>() || goalTy->is<ErrorType>())
          return expr;

        // "Stringifying" casts are a special case. To be
        // eligible, the childTy must be a PrimitiveType.
        if (goalTy->isStringType() && childTy->is<PrimitiveType>())
          return finalizeCastExpr(expr, childTy->isStringType());
        
        // Casting to anything else : just unify or
        // diagnose if unification fails
        if (getSema().unify(childTy, goalTy))
          return finalizeCastExpr(expr, (childTy == goalTy));

        diagnoseInvalidCast(expr);
        return expr;
      }

      Expr* visitUnaryExpr(UnaryExpr* expr) {
        Expr* child = expr->getExpr();
        Type childTy = child->getType()->getAsBoundRValue();

        // If the type isn't bound, give up.
        if (!childTy) return expr;

        // For any unary operators, we only allow integral types,
        // so check that first.
        if (!childTy->isIntegral()) {
          // Not an integral type -> error.
          // Emit diag if childTy isn't a ErrorType too
          if (!childTy->is<ErrorType>())
            diagnoseInvalidUnaryOpChildType(expr);
          return expr;
        }
        
        // If isIntegral returns true, we can safely assume that childTy is a
        // PrimitiveType instance
        PrimitiveType* primChildTy = childTy->castTo<PrimitiveType>();
        return finalizeUnaryExpr(expr, primChildTy);
      }

      Expr* visitArraySubscriptExpr(ArraySubscriptExpr* expr) {
        // Get child expr and it's type
        Expr* child = expr->getBase();
        Type childTy = child->getType()->getAsBoundRValue();
        // Get idx expr and it's type
        Expr* idxE = expr->getIndex();
        Type idxETy = idxE->getType()->getAsBoundRValue();

        // Unbound type as a idx or child: give up
        if (!(idxETy && childTy))
          return expr;

        // Check that the child is an array type.
        if (!childTy->is<ArrayType>()) {
          // Diagnose with the primary range being the child's range
					if(!childTy->is<ErrorType>())
						diagnoseInvalidArraySubscript(expr, 
							child->getRange(), idxE->getRange());
          return expr;
        }

        // Idx type must be an integral value
        if (!idxETy->isIntegral()) {
          // Diagnose with the primary range being the idx's range
					if(!childTy->is<ErrorType>())
						diagnoseInvalidArraySubscript(expr,
							idxE->getRange(), child->getRange());
          return expr;
        }

        // Additionally, check that the index is not a float type. If it
        // is, emit a warning about an implicit integral downcast.
        if (idxETy->isFloatType()) {
          Type intTy = PrimitiveType::getInt(getCtxt());
          warnImplicitIntegralDowncast(idxETy, intTy, idxE->getRange());
        }
        
        // Finalize it
        return finalizeArraySubscriptExpr(expr, childTy);
      }

      Expr* visitMemberOfExpr(MemberOfExpr*) {
        fox_unimplemented_feature("MemberOfExpr TypeChecking");
      }

      Expr* visitDeclRefExpr(DeclRefExpr*) {
        fox_unimplemented_feature("DeclRefExpr TypeChecking");
      }

      Expr* visitFunctionCallExpr(FunctionCallExpr*) {
        fox_unimplemented_feature("FunctionCallExpr TypeChecking");
      }
      
      // Trivial literals: the expr's type is simply the corresponding
      // type. Int for a Int literal, etc.
      Expr* visitCharLiteralExpr(CharLiteralExpr* expr) {
        expr->setType(PrimitiveType::getChar(getCtxt()));
        return expr;
      }

      Expr* visitIntegerLiteralExpr(IntegerLiteralExpr* expr) {
        expr->setType(PrimitiveType::getInt(getCtxt()));
        return expr;
      }

      Expr* visitFloatLiteralExpr(FloatLiteralExpr* expr) {
        expr->setType(PrimitiveType::getFloat(getCtxt()));
        return expr;
      }

      Expr* visitBoolLiteralExpr(BoolLiteralExpr* expr) {
        expr->setType(PrimitiveType::getBool(getCtxt()));
        return expr;
      }

      Expr* visitStringLiteralExpr(StringLiteralExpr* expr) {
        expr->setType(PrimitiveType::getString(getCtxt()));
        return expr;
      }

      Expr* visitArrayLiteralExpr(ArrayLiteralExpr* expr) {
        if (expr->getSize() != 0)
          return checkNonEmptyArrayLiteralExpr(expr);
        else
          return finalizeEmptyArrayLiteral(expr);
      }

      //----------------------------------------------------------------------//
      // Helper checking methods
      //----------------------------------------------------------------------//
      // Various semantics-related helper methods 
      //----------------------------------------------------------------------//

      // visitArrayLiteralExpr helper

      // Typechecks a non empty array literal and deduces it's type.
      Expr* checkNonEmptyArrayLiteralExpr(ArrayLiteralExpr* expr) {
        assert(expr->getSize() && "Size must be >0");

        // The bound type proposed by unifying the other concrete/bound
        // types inside the array.
        Type boundTy;
        // The type used by unbounds elemTy
        Type unboundTy;

        for (auto& elem : expr->getExprs()) {
          Type elemTy = elem->getType();

          if (elemTy->is<ErrorType>()) return expr;

          // Special logic for unbound types
          if (!elemTy->isBound()) {
            // Set unboundTy & continue for first loop
            if (!unboundTy)
              unboundTy = elemTy;
            // Else, just unify.
            else if (!getSema().unify(unboundTy, elemTy)) {
              diagnoseHeteroArrLiteral(expr, elem);
              return expr;
            }
            continue;
          }

          // From this point, elemTy is guaranteed to be a bound type
          // First loop, set boundTy & continue.
          if (!boundTy) {
            boundTy = elemTy;
            continue;
          }

          // Next iterations: Unify elemTy with the bound proposed type.
          if (!getSema().unify(boundTy, elemTy)) {
            diagnoseHeteroArrLiteral(expr, elem);
            return expr;
          }

          // Set boundTy to the highest ranking type of elemTy and boundTy
          boundTy = Sema::getHighestRankedTy(elemTy, boundTy);
          assert(boundTy &&
            "Null highest ranked type but unification succeeded?");
        }
        Type proper;

        // Check if we have an unboundTy and/or a boundTy
        if (unboundTy && boundTy) {
          if (!getSema().unify(unboundTy, boundTy)) {
            // FIXME: A more specific diagnostic might be needed here.
            diagnoseHeteroArrLiteral(expr, nullptr);
            return expr;
          }
          proper = boundTy; 
        }
        else if (boundTy) proper = boundTy;
        else if (unboundTy) proper = unboundTy;
        else fox_unreachable("Should have at least a boundTy or unboundTy set");
        assert(proper && "the proper type shouldn't be null at this stage");
        // The type of the expr is an array of the proper type.
        expr->setType(ArrayType::get(getCtxt(), proper));
        return expr;
      }

      // Typecheck a basic binary expression that requires both operands
      // to be integral types. This includes multiplicative/additive/exponent
      // operations (except concatenation).
      //  \param lhsTy The type of the LHS as a Bound RValue (must not be null)
      //  \param rhsTy The type of the RHS as a Bound RValue (must not be null)
      Expr*
      checkBasicIntegralBinaryExpr(BinaryExpr* expr, Type lhsTy, Type rhsTy) {
        assert((expr->isAdditive() 
             || expr->isExponent() 
             || expr->isMultiplicative()) && "wrong function!");
        
        // Check that lhs and rhs are both integral types.
        if (!(lhsTy->isIntegral() && rhsTy->isIntegral())) {
          diagnoseInvalidBinaryExprOperands(expr);
          return expr;
        }

        // The expression type is the highest ranked type between lhs & rhs
        Type highest = getSema().getHighestRankedTy(lhsTy, rhsTy);
        assert(highest && "Both types are integral, so getHighestRankedTy "
          "shoudln't return a null value");

        // Set the type of the expression to the highest ranked type
        // unless it's a boolean, then uprank it.
        expr->setType(uprankIfBoolean(highest));
        return expr;
      }

      // Returns true if this combination of operator/types
      // is eligible to be a concatenation operation
      //  \param op The operation kind
      bool canConcat(BinaryExpr::OpKind op, Type lhsTy, Type rhsTy) {
        // It is eligible if the operator is a '+'
        if (op == BinaryExpr::OpKind::Add) {
          // and the LHS and RHS are string or char types.
          bool lhsOk = lhsTy->isCharType() || lhsTy->isStringType();
          bool rhsOk = rhsTy->isCharType() || rhsTy->isStringType();
          return lhsOk && rhsOk;
        }
        return false;
      }

      // Typechecks an assignement operation
      //  \param lhsTy The type of the LHS (must not be null)
      //  \param rhsTy The type of the RHS (must not be null)
      Expr* checkAssignementBinaryExpr(BinaryExpr* expr, Type /*lhsTy*/, Type /*rhsTy*/) {
        assert(expr->isAssignement() && "wrong function!");
        // Leave unimplemented for now (at least until
        // name binding is done)
        fox_unimplemented_feature(__func__);
      }

      // Typechecks a comparative operation
      //  \param lhsTy The type of the LHS (must not be null)
      //  \param rhsTy The type of the RHS (must not be null)
      Expr* 
      checkComparisonBinaryExpr(BinaryExpr* expr, Type lhsTy, Type rhsTy) {
        assert(expr->isComparison() && "wrong function!");

        if (!getSema().unify(lhsTy, rhsTy)) {
          diagnoseInvalidBinaryExprOperands(expr);
          return expr;
        }

        // For ranking comparisons, only allow Primitives types as LHS/RHS
        if (expr->isRankingComparison()) {
          // FIXME: Maybe disallow char comparisons? Depends on how it's going
          // to be implemented... It needs to be intuitive!
          if (!(lhsTy->is<PrimitiveType>() && rhsTy->is<PrimitiveType>())) {
            diagnoseInvalidBinaryExprOperands(expr);
            return expr;
          }
        }

        return finalizeBooleanExpr(expr);
      }
      
      // Typechecks a logical and/or operation
      //  \param lhsTy The type of the LHS (must not be null)
      //  \param rhsTy The type of the RHS (must not be null)
      Expr* checkLogicalBinaryExpr(BinaryExpr* expr, Type lhsTy, Type rhsTy) {
        assert(expr->isLogical() && "wrong function!");

        // for logical AND and OR operations, only allow integral
        // types for the LHS and RHS
        if (!(lhsTy->isIntegral() && rhsTy->isIntegral())) {
          diagnoseInvalidBinaryExprOperands(expr);
          return expr;
        }

        return finalizeBooleanExpr(expr);
      }

      //----------------------------------------------------------------------//
      // Other helper methods
      //----------------------------------------------------------------------//
      // Various helper methods unrelated to semantics
      //----------------------------------------------------------------------//
      
      // If type is a boolean, returns the int type, else returns the argument.
      // Never returns null.
      Type uprankIfBoolean(Type type) {
        assert(type && "Type cannot be null!");
        if (type->isBoolType())
          return PrimitiveType::getInt(getCtxt());
        return type;
      }

  };

  // ExprFinalizer, which rebuilds types to remove
  // CellTypes.
  //
  // Visit methods return Type objects. They return null Types
  // if the finalization failed for this expr.
  class ExprFinalizer : TypeVisitor<ExprFinalizer, Type>, ASTWalker {
    ASTContext& ctxt_;
    DiagnosticEngine& diags_;

    public:
      ExprFinalizer(ASTContext& ctxt, DiagnosticEngine& diags) :
        ctxt_(ctxt), diags_(diags) {

      }

      Expr* finalize(Expr* expr) {
        return walk(expr);
      }

      Expr* handleExprPost(Expr* expr) {
        Type type = expr->getType();
        assert(type && "Untyped expr");

        // Visit the type
        type = visit(type);
        
        // If the type is nullptr, this inference failed
        // because of a lack of substitution somewhere.
        // Set the type to ErrorType, diagnose it and move on.
        if (!type) {
          diags_.report(DiagID::sema_failed_infer, expr->getRange());
          type = ErrorType::get(ctxt_);
        }

        expr->setType(type);
        return expr;
      }

      /*
        How to write a visit method:
          -> if the type is a "container" (has a pointer to
             another type inside it) and after calling visit
             the type changed, rebuild the type with the returne type.
             If the element type is ErrorType, don't rebuild and just
             return the ErrorType.
      */

      Type visitPrimitiveType(PrimitiveType* type) {
        return type;
      }

      Type visitArrayType(ArrayType* type) {
        if (Type elem = visit(type->getElementType())) {
          if (elem->is<ErrorType>())
            return elem;
          if (elem != type->getElementType())
            return ArrayType::get(ctxt_, elem);
          return type;
        }
        return nullptr;
      }

      Type visitLValueType(LValueType* type) {
        if (Type elem = visit(type->getType())) {
          if (elem->is<ErrorType>())
            return elem;
          if (elem != type->getType())
            return LValueType::get(ctxt_, elem);
          return type;
        }
        return nullptr;
      }

      Type visitCellType(CellType* type) {
        if (Type sub = type->getSubst())
          return visit(sub);
        return nullptr;
      }

      Type visitErrorType(ErrorType* type) {
        return type;
      }
  };
} // End anonymous namespace

std::pair<bool, Expr*> Sema::typecheckExpr(Expr* expr) {
  assert(expr && "null input");
  expr = ExprChecker(*this).check(expr);
  assert(expr && "Expr is null post typechecking");
  expr = ExprFinalizer(ctxt_, diags_).finalize(expr);
  assert(expr && "Expr is null post finalization");
  // Success is if the type of the expression isn't ErrorType.
  return { !expr->getType()->is<ErrorType>(), expr };
}
