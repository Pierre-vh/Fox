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
#include "Fox/AST/ASTContext.hpp"
#include "Fox/AST/ASTVisitor.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"

#include <utility>
#include <iostream>

using namespace fox;


// Expression checker: Classic visitor, the visitXXX functions
// all check a single node. They do not orchestrate visitation of
// the children, because that is done by the ASTWalker logic
//
// Every visitation method return a pointer to an Expr*, which is the current 
// expr
// OR the expr that should take it's place. This can NEVER be null.
class Sema::ExprChecker : Checker, ExprVisitor<ExprChecker, Expr*>,  ASTWalker {
  using Inherited = ExprVisitor<ExprChecker, Expr*>;
  friend Inherited;
  public:
    ExprChecker(Sema& sema) : Checker(sema) {}

    Expr* check(Expr* expr) {
      Expr* e = walk(expr);
      assert(e && "expression is nullptr after the walk");
      return e;
    }

  private:
    //----------------------------------------------------------------------//
    // Diagnostic methods
    //----------------------------------------------------------------------//
    // The diagnose family of methods are designed to print the most relevant
    // diagnostics for a given situation.
    //----------------------------------------------------------------------//

    // (Note) example: "'foo' declared here with type 'int'"
    void noteIsDeclaredHereWithType(ValueDecl* decl) {
      Identifier id = decl->getIdentifier();
      SourceRange range = decl->getIdentifierRange();
      assert(id && range && "ill formed ValueDecl");
      getDiags().report(DiagID::sema_declared_here_with_type, range)
        .addArg(id).addArg(decl->getType());
    }

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
    void warnRedundantCast(CastExpr* expr, Type toType) {
      SourceRange range = expr->getCastTypeLoc().getRange();
      getDiags()
        .report(DiagID::sema_useless_cast_redundant, range)
        .addArg(toType)
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
        .addArg(rhsTy)
        .setExtraRange(exprRange);
    }

    // Diagnoses an undeclared identifier
    void diagnoseUndeclaredIdentifier(SourceRange range, Identifier id) {
      getDiags().report(DiagID::sema_undeclared_id, range).addArg(id);
    }

    // Diagnoses an ambiguous identifier
    void diagnoseAmbiguousIdentifier(SourceRange range, Identifier id,
      const LookupResult& results) {
      // First, display the "x" is ambiguous error
      getDiags().report(DiagID::sema_ambiguous_ref, range).addArg(id);
      // Now, iterate over the lookup results and emit notes
      // for each candidate.
      assert(results.isAmbiguous());
      for(auto result : results) {
        getDiags().report(DiagID::sema_potential_candidate_here, 
          result->getIdentifierRange());
      }
    }

    void diagnoseUnassignableExpr(BinaryExpr* expr) {
      assert(expr->isAssignement());
      SourceRange lhsRange = expr->getLHS()->getRange();
      SourceRange opRange = expr->getOpRange();
      getDiags().report(DiagID::sema_unassignable_expr, lhsRange)
        .setExtraRange(opRange);
    }

    void diagnoseInvalidAssignement(BinaryExpr* expr, Type lhsTy, Type rhsTy) {
      assert(expr->isAssignement());
      SourceRange lhsRange = expr->getLHS()->getRange();
      SourceRange rhsRange = expr->getRHS()->getRange();
      // Diag is (roughly) "can't assign a value of type (lhs) to type (rhs)
      getDiags().report(DiagID::sema_invalid_assignement, rhsRange)
        .setExtraRange(lhsRange)
        .addArg(rhsTy).addArg(lhsTy);
    }

    // Diagnoses a variable being used inside it's own initial value.
    void diagnoseVarInitSelfRef(VarDecl* decl, 
      UnresolvedDeclRefExpr* udre) {
      SourceRange range = udre->getRange();
      SourceRange extra = decl->getIdentifierRange();
      getDiags().report(DiagID::sema_var_init_self_ref, range)
        .setExtraRange(extra);
    }

    void diagnoseExprIsNotAFunction(Expr* callee) {
      SourceRange range = callee->getRange();
      Type ty = callee->getType();
      getDiags().report(DiagID::sema_expr_isnt_func, range)
        .addArg(ty);
    }

    void diagnoseArgcMismatch(CallExpr* call, std::size_t argsProvided, 
      std::size_t argsExpected) {
      assert(argsProvided != argsExpected);
      assert((call->getCallee() != nullptr) && "no callee");
      DeclRefExpr* callee = dyn_cast<DeclRefExpr>(call->getCallee());

      // For now, only a DeclRefExpr can have a FunctionType. But if that
      // changes in the future, remove this assert and add alternative diags.
      assert(callee && "callee isn't a DeclRefExpr");

      DiagID diag;
      // Use the most appropriate diagnostic based on the situation
      if(argsProvided == 0) 
        diag = DiagID::sema_cannot_call_with_no_args;
      else if(argsProvided < argsExpected) 
        diag = DiagID::sema_not_enough_args_in_call_to;
      else 
        diag = DiagID::sema_too_many_args_in_call_to;

      // Report the diagnostic
      getDiags().report(diag, callee->getRange())
        .addArg(callee->getDecl()->getIdentifier());
      // Also emit a "is declared here with type" note.
      noteIsDeclaredHereWithType(callee->getDecl());
    }

    // Diagnoses a bad function call where the types didn't match
    void diagnoseBadFunctionCall(CallExpr* call) {
      assert((call->getCallee() != nullptr) && "no callee");
      assert(call->numArgs() && "numArgs cannot be zero!");
      DeclRefExpr* callee = dyn_cast<DeclRefExpr>(call->getCallee());

      // For now, only a DeclRefExpr can have a FunctionType. But if that
      // changes in the future, remove this assert and add alternative diags.
      assert(callee && "callee isn't a DeclRefExpr");

      // Retrieve a user-friendly presentation of the args
      std::string argsAsStr = getArgsAsString(call);

      // Get the args range
      SourceRange argsRange = call->getArgsRange();
      assert(argsRange && "argsRange is invalid in CallExpr with a non-zero "
        "number of arguments");

      getDiags().report(DiagID::sema_cannot_call_func_with_args, callee->getRange())
        .addArg(callee->getDecl()->getIdentifier())
        .addArg(argsAsStr)
        .setExtraRange(argsRange);

      noteIsDeclaredHereWithType(callee->getDecl());
    }

    void diagnoseFunctionTypeInArrayLiteral(ArrayLiteralExpr* lit, Expr* fn) {
      getDiags().report(DiagID::sema_fnty_in_array, fn->getRange())
        // Maybe displaying the whole array is too much? I think it's great
        // because it gives some context, but maybe I'm wrong.
        .setExtraRange(lit->getRange());
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
        // Redundant casts are useless
        expr->markAsUselesss();
        // Warn the user
        Type type = expr->getCastType();
        warnRedundantCast(expr, type);
      }

      // Else, the Expr's type is simply the castgoal.
      expr->setType(expr->getCastType());
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
      assert((expr->numElems() == 0) && "Only for empty Array Literals");
      // For empty array literals, the type is going to be a fresh
      // celltype inside an Array : Array(CellType(null))
      Type type = CellType::create(getCtxt());
      type = ArrayType::get(getCtxt(), type); 
      expr->setType(type);
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

    Expr* finalizeReferenceToValueDecl(UnresolvedDeclRefExpr* udre, 
      ValueDecl* found) {
      assert(found);

      // Check that we aren't using a variable inside it's own initial value
      if(found->isChecking()) {
        if(VarDecl* var = dyn_cast<VarDecl>(found)) {
          // Currently, that should *always* mean that we're inside
          // the initializer, so we're going to assert that it's
          // the case. 
          // If one day Semantic analysis becomes more complex
          // and the assertions are triggered in valid code, replace them
          // by conditions.
          Expr* init = var->getInitExpr();
          assert(init);
          assert(init->getRange().contains(udre->getRange()));
          diagnoseVarInitSelfRef(var, udre);
          // This is an error, so just return the UnresolvedDeclRefExpr
          return udre;
        }
      }

      // Resolved DeclRef
      DeclRefExpr* resolved = 
        DeclRefExpr::create(getCtxt(), found, udre->getRange());
      
      // Assign it's type
      Type valueType = found->getType();
      assert(valueType && "ValueDecl doesn't have a Type!");
      // If it's a non const ValueDecl, wrap it in a LValue
      if(!found->isConst()) {
        assert(!isa<FuncDecl>(found) && "FuncDecl are always const!");
        valueType = LValueType::get(getCtxt(), valueType);
      }

      resolved->setType(valueType);
      return resolved;
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

    virtual bool handleDeclPre(Decl*) {
      fox_unreachable("Illegal node kind");
    }

    //----------------------------------------------------------------------//
    // "visit" methods
    //----------------------------------------------------------------------//
    // Theses visit() methods will perform the necessary tasks to check a
    // single expression node. In trivial/simple cases, theses methods will do
    // everything needed to typecheck the expr (diagnose, finalize, etc), but
    // they may also delegate the work to some check/finalize methods in more
    // complex cases.
    //
    // Theses methods will never call visit on the Expr's children. Children
    // visitation is only done through the ASTWalker, as we always want to
    // visit the Expression tree in postorder.
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
        // are checked by checkBasicNumericBinaryExpr, except
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
          return checkBasicNumericBinaryExpr(expr, lhsTy, rhsTy);
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
      std::tie(childTy, goalTy) = Sema::unwrapAll(childTy, goalTy);

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

      // For any unary operators, we only allow numeric types,
      // so check that first.
      if (!childTy->isNumeric()) {
        // Not a numeric type -> error.
        // Emit diag if childTy isn't a ErrorType too
        if (!childTy->is<ErrorType>())
          diagnoseInvalidUnaryOpChildType(expr);
        return expr;
      }
        
      // If isNumeric returns true, we can safely assume that childTy is a
      // PrimitiveType instance
      PrimitiveType* primChildTy = childTy->castTo<PrimitiveType>();
      return finalizeUnaryExpr(expr, primChildTy);
    }

    Expr* visitArraySubscriptExpr(ArraySubscriptExpr* expr) {
      // Get child expr and it's type
      Expr* child = expr->getBase();
      Type childTy = child->getType()->getRValue();
      // Get idx expr and it's type
      Expr* idxE = expr->getIndex();
      Type idxETy = idxE->getType()->getAsBoundRValue();

      // Unbound type as a idx: give up
      if (!idxETy)
        return expr;

      Type subscriptType;
      // Check that the child is an array type
      if (childTy->is<ArrayType>()) {
        subscriptType = childTy->castTo<ArrayType>()->getElementType();
        assert(subscriptType && "ArrayType had no element type!");
      }
      // Or a string
      else if(childTy->isStringType())
        subscriptType = PrimitiveType::getChar(getCtxt());
      else {
        // Diagnose with the primary range being the child's range
				if(!childTy->is<ErrorType>())
					diagnoseInvalidArraySubscript(expr, 
						child->getRange(), idxE->getRange());
        return expr;
      }

      // Idx type must be an numeric value, but can't be a float
      if ((!idxETy->isNumeric()) || idxETy->isDoubleType()) {
        // Diagnose with the primary range being the idx's range
				if(!childTy->is<ErrorType>())
					diagnoseInvalidArraySubscript(expr,
						idxE->getRange(), child->getRange());
        return expr;
      }
        
      // Set type + return
      assert(subscriptType);
      expr->setType(subscriptType);
      return expr;
    }

    Expr* visitMemberOfExpr(MemberOfExpr*) {
      fox_unimplemented_feature("MemberOfExpr TypeChecking");
    }

    Expr* visitUnresolvedDeclRefExpr(UnresolvedDeclRefExpr* expr) {
      Identifier id = expr->getIdentifier();
      SourceRange range = expr->getRange();
      LookupResult results;
      getSema().doUnqualifiedLookup(results, id, expr->getBegin());
      // No results -> undeclared identifier
      if(results.isEmpty()) {
        diagnoseUndeclaredIdentifier(range, id);
        return expr;
      }
      // Ambiguous 
      if(results.isAmbiguous()) {
        diagnoseAmbiguousIdentifier(range, id, results);
        return expr;
      }
      // Correct
      NamedDecl* decl = results.getIfSingleResult();
      assert(decl && "not ambiguous, not empty, but not single?");
      if(ValueDecl* valueDecl = dyn_cast<ValueDecl>(decl)) 
        return finalizeReferenceToValueDecl(expr, valueDecl);
      // For now, every NamedDecl is also a ValueDecl
      fox_unreachable("unknown NamedDecl kind");
    }

    Expr* visitDeclRefExpr(DeclRefExpr*) {
      // Shouldn't happen at all.
      fox_unreachable("Expr checked twice!");
    }

    Expr* visitCallExpr(CallExpr* expr) {
      Expr* callee = expr->getCallee();
      Type calleeTy = callee->getType();
      
      if(!calleeTy->is<FunctionType>()) {
        if(!calleeTy->is<ErrorType>())
          diagnoseExprIsNotAFunction(callee);
        return expr;
      }

      FunctionType* fnTy = calleeTy->castTo<FunctionType>();

      // Check arg count
      std::size_t callArgc = expr->numArgs();
      std::size_t expectedArgc = fnTy->numParams();
      if(callArgc != expectedArgc) {
        diagnoseArgcMismatch(expr, callArgc, expectedArgc);
        return expr;
      }

      // Check arg types
      for(std::size_t idx = 0; idx < callArgc; idx++) {
        Type expected = fnTy->getParamType(idx);
        Type got = expr->getArg(idx)->getType();
        assert(expected && got && "types cant be nullptrs!");
        if(!getSema().unify(expected, got, /*allowDowncast*/ false)) {
          diagnoseBadFunctionCall(expr);
          return expr;
        }
      }

      // Call should be ok. The type of the CallExpr is the return type
      // of the function.
      Type ret = fnTy->getReturnType();
      assert(ret && "types cant be nullptrs!");
      expr->setType(ret);
      return expr;
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

    Expr* visitDoubleLiteralExpr(DoubleLiteralExpr* expr) {
      expr->setType(PrimitiveType::getDouble(getCtxt()));
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
      if (expr->numElems() != 0)
        return checkNonEmptyArrayLiteralExpr(expr);
      else
        return finalizeEmptyArrayLiteral(expr);
    }

    //----------------------------------------------------------------------//
    // Helper checking methods
    //----------------------------------------------------------------------//
    // Various semantics-related helper methods 
    //----------------------------------------------------------------------//

    // visitArrayLiteralExpr helpers

    bool checkIfLegalWithinArrayLiteral(ArrayLiteralExpr* lit, Expr* expr) {
      Type ty = expr->getType()->getAsBoundRValue();
      // unbound types are ok
      if(!ty) return true;
      // check if not function type
      if(ty->is<FunctionType>()) {
        diagnoseFunctionTypeInArrayLiteral(lit, expr);
        return false;
      }
      return true;
    }

    // Typechecks a non empty array literal and deduces it's type.
    Expr* checkNonEmptyArrayLiteralExpr(ArrayLiteralExpr* expr) {
      assert(expr->numElems() && "Size must be >0");

      // The bound type proposed by unifying the other concrete/bound
      // types inside the array.
      Type boundTy;
      // The type used by unbounds elemTy
      Type unboundTy;
      // Set to false if the ArrayLiteral is not valid
      bool isValid = true;

      for (auto& elem : expr->getExprs()) {
        // Skip the elem & mark the array literal as invalid
        if(!checkIfLegalWithinArrayLiteral(expr, elem)) {
          isValid = false;
          continue;
        }

        Type elemTy = elem->getType();

        if (elemTy->is<ErrorType>()) {
          isValid = false;
          continue;
        }

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
      else {
        // If the expr isn't valid, this is a normal situation.
        if(!isValid)
          return expr;
        // If it's valid, we have a bug!
        fox_unreachable("Should have at least a boundTy or unboundTy set");
      }

      assert(proper && "the proper type shouldn't be null at this stage");

      // Set the type only if the expr is valid, because if it's not
      // valid it should be marked as "ErrorType"
      if(isValid)
        // The type of the expr is an array of the proper type.
        expr->setType(ArrayType::get(getCtxt(), proper));

      // Return the expr
      return expr;
    }

    // Typecheck a basic binary expression that requires both operands
    // to be numeric types. This includes multiplicative/additive/exponent
    // operations (except concatenation).
    //  \param lhsTy The type of the LHS as a Bound RValue (must not be null)
    //  \param rhsTy The type of the RHS as a Bound RValue (must not be null)
    Expr*
    checkBasicNumericBinaryExpr(BinaryExpr* expr, Type lhsTy, Type rhsTy) {
      assert((expr->isAdditive() 
            || expr->isExponent() 
            || expr->isMultiplicative()) && "wrong function!");
        
      // Check that lhs and rhs are both numeric types.
      if (!(lhsTy->isNumeric() && rhsTy->isNumeric())) {
        diagnoseInvalidBinaryExprOperands(expr);
        return expr;
      }

      // The expression type is the highest ranked type between lhs & rhs
      Type highest = getSema().getHighestRankedTy(lhsTy, rhsTy);
      assert(highest && "Both types are numeric, so getHighestRankedTy "
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
    Expr* checkAssignementBinaryExpr(BinaryExpr* expr, Type lhsTy, Type rhsTy) {
      assert(expr->isAssignement() && "wrong function!");
      
      if(!lhsTy->isAssignable()) {
        diagnoseUnassignableExpr(expr);
        return expr;
      }
      
      // For now, in the langage, the only kind of Expr that should be able
      // to carry an LValue is a DeclRefExpr, so check that our LHS is 
      // indeed that, just as a sanity check.
      assert(isa<DeclRefExpr>(expr->getLHS()) && "Only DeclRefExprs can be "
        "LValues!");

      // Get the bound RValue version of the LHS.
      lhsTy = lhsTy->getAsBoundRValue();
      // Some more sanity checks:
        // Can't have unbound LValues
      assert(lhsTy && "DeclRefExpr has a LValue to an unbound type?");
        // Can't assign to a function
      assert((!lhsTy->is<FunctionType>()) && "Assigning to a function?");

      // Ignore the LValue on the RHS.
      rhsTy = rhsTy->getRValue();

      // Unify
      if(!getSema().unify(lhsTy, rhsTy, /*allowDowncast*/ false)) {
        // Type mismatch
        diagnoseInvalidAssignement(expr, lhsTy, rhsTy);
        return expr;
      }

      // Everything's fine, the type of the expr is the type of it's RHS.
      expr->setType(rhsTy);
      return expr;
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

      // for logical AND and OR operations, only allow numeric
      // types for the LHS and RHS
      if (!(lhsTy->isNumeric() && rhsTy->isNumeric())) {
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

    // Returns a string containing the arguments passed to a CallExpr,
    // in round brackets, separated by commas.
    //  e.g. CallExpr: foo(3,[],"s")
    //       Result: (int, [any], string)
    std::string getArgsAsString(CallExpr* call) {
      std::stringstream ss;
      ss << "(";
      bool first = true;
      for(Expr* arg : call->getArgs()) {
        if(first) first = false;
        else ss << ",";
        ss << arg->getType()->toString();
      }
      ss << ")";
      return ss.str();
    }
};

namespace {
  // ExprFinalizer, which rebuilds types to remove
  // CellTypes.
  //
  // Visit methods return Type objects. They return null Types
  // if the finalization failed for this expr.
  class ExprFinalizer : TypeVisitor<ExprFinalizer, Type>, ASTWalker {
    using Inherited = TypeVisitor<ExprFinalizer, Type>;
    friend Inherited;
    ASTContext& ctxt_;
    DiagnosticEngine& diags_;
    public:
      ExprFinalizer(ASTContext& ctxt) : ctxt_(ctxt), diags_(ctxt.diagEngine) {}

      Expr* finalize(Expr* expr) {
        Expr* e = walk(expr);
        assert(e && "expr is null post walk");
        return e;
      }

      std::pair<Expr*, bool> handleExprPre(Expr* expr) {
        Type type = expr->getType();
        assert(type && "Expr has a null type!");

        // Visit the type
        type = visit(type);
        bool shouldVisitChildren = true;
        // If the type is nullptr, this inference failed
        // because of a lack of substitution somewhere.
        // Set the type to ErrorType, diagnose it and move on.
        if (!type) {
          diags_.report(DiagID::sema_failed_infer, expr->getRange());
          type = ErrorType::get(ctxt_);
          shouldVisitChildren = false;
        }

        expr->setType(type);
        return {expr, shouldVisitChildren};
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
        // Assert that we have emitted at least 1 error if
        // we have a ErrorType present in the hierarchy.
        assert(ctxt_.hadErrors());
        return type;
      }

      Type visitFunctionType(FunctionType* type) {
        // Get return type
        Type returnType = visit(type->getReturnType());
        if(!returnType) return nullptr;
        // Get Param types
        SmallVector<Type, 4> paramTypes;
        for(auto param : type->getParamTypes()) {
          if(Type t = visit(param))
            paramTypes.push_back(t);
          else 
            return nullptr;
        }
        // Recompute if needed
        if(!type->isSame(paramTypes, returnType))
          return FunctionType::get(ctxt_, paramTypes, returnType);
        return type;
      }
  };
} // End anonymous namespace

Expr* Sema::typecheckExpr(Expr* expr) {
  assert(expr && "null input");
  expr = ExprChecker(*this).check(expr);
  expr = ExprFinalizer(ctxt_).finalize(expr);
  // Success is if the type of the expression isn't ErrorType.
  return expr;
}

bool Sema::typecheckExprOfType(Expr*& expr, Type type, bool allowDowncast) {
  assert(expr && "null input");

  expr = ExprChecker(*this).check(expr);
  bool success = unify(type, expr->getType(), allowDowncast);
  expr = ExprFinalizer(ctxt_).finalize(expr);

  // Don't allow downcasts
  if(success)
    success = !isDowncast(expr->getType(), type);

  return success;
}

bool Sema::typecheckCondition(Expr*& expr) {
  expr = ExprChecker(*this).check(expr);
  Type boolTy = PrimitiveType::getBool(getASTContext());
  bool success = unify(expr->getType(), boolTy);
  expr = ExprFinalizer(ctxt_).finalize(expr);
  return success && !(expr->getType()->is<ErrorType>());
 }