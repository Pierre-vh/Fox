//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
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
    //
    // Generally speaking, theses methods won't emit diagnostics if 
    // ill formed types are involved, because theses have either:
    //  - been diagnosed already
    //  - will be diagnosed at finalization
    //----------------------------------------------------------------------//

    // (Note) example: "'foo' declared here with type 'int'"
    void noteIsDeclaredHereWithType(ValueDecl* decl) {
      Identifier id = decl->getIdentifier();
      SourceRange range = decl->getIdentifierRange();
      Type declType = decl->getValueType();

      if(!Sema::isWellFormed(declType)) return;

      assert(id && range && "ill formed ValueDecl");
      getDiags().report(DiagID::sema_declared_here_with_type, range)
        .addArg(id).addArg(declType);
    }

    // (Error) Diagnoses an invalid cast 
    void diagnoseInvalidCast(CastExpr* expr) {
      SourceRange range = expr->getCastTypeLoc().getRange();
      Type childTy = expr->getExpr()->getType();
      Type goalTy = expr->getCastTypeLoc().getType();

      if(!Sema::isWellFormed({childTy, goalTy})) return;

      getDiags()
        .report(DiagID::sema_invalid_cast, range)
        .addArg(childTy)
        .addArg(goalTy)
        .setExtraRange(expr->getExpr()->getRange());
    }

    // (Warning) Diagnoses a redudant cast (when the
    // cast goal and the child's type are equal)
    void warnRedundantCast(CastExpr* expr, TypeLoc castTL) {
      Type castTy = castTL.getType();
      if(!Sema::isWellFormed(castTy)) return;

      SourceRange range = expr->getCastTypeLoc().getRange();

      getDiags()
        .report(DiagID::sema_useless_cast_redundant, castTL.getRange())
        .addArg(castTy)
        .setExtraRange(expr->getExpr()->getRange());
    }

    void diagnoseHeteroArrLiteral(ArrayLiteralExpr* expr, Expr* faultyElem,
      Type supposedType) {
      assert(faultyElem && "no element pointed");
      getDiags()
        // Precise error loc is the first element that failed the inferrence,
        // extended range is the whole arrayliteral's.
        .report(DiagID::sema_unexpected_elem_in_arrlit, faultyElem->getRange())
        .addArg(faultyElem->getType())
        // Sometimes, the supposed type might contain a type variable.
        // Try to simplify the type to produce a better diagnostic!
        .addArg(getSema().trySimplify(supposedType))
        .setExtraRange(expr->getRange());
    }

    void diagnoseInvalidUnaryOpChildType(UnaryExpr* expr) {
      Expr* child = expr->getExpr();
      Type childTy = child->getType();

      if(!Sema::isWellFormed(childTy)) return;

      getDiags()
        .report(DiagID::sema_unaryop_bad_child_type, expr->getOpRange())
        // Use the child's range as the extra range.
        .setExtraRange(child->getRange())
        .addArg(expr->getOpSign()) // %0 is the operator's sign as text
        .addArg(childTy); // %1 is the type of the child
    }

    void diagnoseInvalidArraySubscript(ArraySubscriptExpr* expr,
                                       SourceRange range, SourceRange extra) {
      Expr* child = expr->getBase();
      Type childTy = child->getType();

      Expr* idxE = expr->getIndex();
      Type idxETy = idxE->getType();

      if(!Sema::isWellFormed({childTy, idxETy})) return;

      getDiags()
        .report(DiagID::sema_arrsub_invalid_types, range)
        // %0 is subscripted value's type, %1 is the index's expr type;
        .addArg(childTy)
        .addArg(idxETy)
        .setExtraRange(extra);
    }

    void diagnoseInvalidBinaryExpr(BinaryExpr* expr) {
      SourceRange opRange = expr->getOpRange();
      SourceRange exprRange = expr->getRange();
      Type lhsTy = expr->getLHS()->getType();
      Type rhsTy = expr->getRHS()->getType();

      if(!Sema::isWellFormed({lhsTy, rhsTy})) return;

      // Use the specific diagnostic for assignements
      if(expr->isAssignement()) 
        return diagnoseInvalidAssignement(expr, lhsTy, rhsTy);

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

      if(!Sema::isWellFormed({lhsTy, rhsTy})) return;

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

      if(!Sema::isWellFormed(ty)) return;

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
      TypeLoc castTL = expr->getCastTypeLoc();

      if (isRedundant) { 
        expr->markAsUselesss();
        warnRedundantCast(expr, castTL);
      }

      expr->setType(castTL.getType());
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
          // Always int or float
          assert(!childTy->isBoolType());
          expr->setType(childTy);
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
      // TypeVariable inside an Array. e.g. [$T0]
      Type type = getSema().createNewTypeVariable();
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
      Type valueType = found->getValueType();
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

      // Check that the types are well formed. If they aren't, don't
      // bother checking.
      if (!Sema::isWellFormed({lhsTy, rhsTy}))
        return expr;

      // Handle assignements early, let checkAssignementBinaryExpr do it.
      if (expr->isAssignement())
        return checkAssignementBinaryExpr(expr, lhsTy, rhsTy);

      // For every other operator, we must use the bound RValue version
      // of the types.
      lhsTy = lhsTy->getRValue();
      rhsTy = rhsTy->getRValue();

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
      Type goalTy = expr->getCastTypeLoc().getType();
      std::tie(childTy, goalTy) = Sema::unwrapAll(childTy, goalTy);

      // Check that the types are well formed. If they aren't, don't
      // bother checking.
      if (!Sema::isWellFormed({childTy, goalTy}))
        return expr;

      // "Stringifying" casts are a special case. To be
      // eligible, the childTy must be a PrimitiveType.
      if (goalTy->isStringType() && childTy->is<PrimitiveType>())
        return finalizeCastExpr(expr, childTy->isStringType());
        
      bool isPerfectEquality = false;
      // custom comparator which considers that a and b are
      // equal when they're both numeric or boolean types.
      auto comparator = [&isPerfectEquality](Type a, Type b) {
        if(a == b) {
          isPerfectEquality = true;
          return true;
        }
        // Allow casting between numeric & booleans types.
        return a->isNumericOrBool() && b->isNumericOrBool();
      };

      // Try unification
      if (getSema().unify(childTy, goalTy, comparator))
        return finalizeCastExpr(expr, isPerfectEquality);      

      diagnoseInvalidCast(expr);
      return expr;
    }

    Expr* visitUnaryExpr(UnaryExpr* expr) {
      using UOp = UnaryExpr::OpKind;
      Expr* child = expr->getExpr();
      Type childTy = child->getType()->getRValue();

      // If the type isn't bound, give up.
      if (!childTy) return expr;

      // The expression is valid iff:
      //  -> The child type is a numeric type
      //  OR
      //  -> The child type is a boolean and the operation is a '!' (LNot)
      if (childTy->isNumeric() || 
         (childTy->isBoolType() && (expr->getOp() == UOp::LNot))) {
        // If isNumeric returns true, we can safely assume that childTy is a
        // PrimitiveType instance
        PrimitiveType* primChildTy = childTy->castTo<PrimitiveType>();
        return finalizeUnaryExpr(expr, primChildTy);
      }
        
      diagnoseInvalidUnaryOpChildType(expr);
      return expr;
    }

    Expr* visitArraySubscriptExpr(ArraySubscriptExpr* expr) {
      // Get child expr and it's type
      Expr* child = expr->getBase();
      Type childTy = child->getType()->getRValue();
      // Get idx expr and it's type
      Expr* idxE = expr->getIndex();
      Type idxETy = idxE->getType()->getRValue();

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
			  diagnoseInvalidArraySubscript(expr, child->getRange(), idxE->getRange());
        return expr;
      }

      // Idx type must be an int.
      if (!idxETy->isIntType()) {
        // Diagnose with the primary range being the idx's range
			  diagnoseInvalidArraySubscript(expr, idxE->getRange(), child->getRange());
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
        if(!getSema().unify(expected, got)) {
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
      Type ty = expr->getType()->getRValue();
      assert(ty && "can't be nullptr!");
      // check if not function type
      if(ty->is<FunctionType>()) {
        diagnoseFunctionTypeInArrayLiteral(lit, expr);
        return false;
      }
      // check if not ill formed. If it's ill formed, it has already
      // been diagnosed so just ignore it.
      if(!Sema::isWellFormed(ty))
        return false;
      return true;
    }

    // Typechecks a non empty array literal and deduces it's type.
    Expr* checkNonEmptyArrayLiteralExpr(ArrayLiteralExpr* expr) {
      assert(expr->numElems() && "Size must be >0");

      // The deduced type of the literal
      Type proposedType;

      // Set to false if the ArrayLiteral is considered invalid.
      bool isValid = true;

      for (auto& elem : expr->getExprs()) {
        // Check if the element's type can legally appear inside an
        // array literal. If it can't, skip the elem & mark the
        // literal as invalid
        if(!checkIfLegalWithinArrayLiteral(expr, elem)) {
          isValid = false;
          continue;
        }

        // Retrieve the type as a RValue
        Type elemTy = elem->getType()->getRValue();

        // From this point, elemTy is guaranteed to be a bound type
        // First loop, set the proposed type and continue.
        if (!proposedType) {
          proposedType = elemTy;
          continue;
        }

        // Next iterations: Unify the element's type with the proposed type.
        if (!getSema().unify(proposedType, elemTy)) {
          diagnoseHeteroArrLiteral(expr, elem, proposedType);
          continue;
        }
      }

      // Set the type to an ArrayType of the
      // type if the expr is still considered valid.
      if(isValid)
        expr->setType(ArrayType::get(getCtxt(), proposedType));
      return expr;
    }

    // Typecheck a basic binary expression that involves numeric types. 
    // This includes multiplicative/additive/exponent
    // operations (except concatenation).
    //  \param lhsTy The type of the LHS as a Bound RValue (must not be null)
    //  \param rhsTy The type of the RHS as a Bound RValue (must not be null)
    Expr*
    checkBasicNumericBinaryExpr(BinaryExpr* expr, Type lhsTy, Type rhsTy) {
      assert((expr->isAdditive() 
            || expr->isExponent() 
            || expr->isMultiplicative()) && "wrong function!");
        
      // Check that lhs and rhs unify and that they're both numeric
      // types.
      if(getSema().unify(lhsTy, rhsTy) && 
        (lhsTy->isNumeric() && rhsTy->isNumeric())) {
        // lhsTy and rhsTy are equal
        assert((lhsTy == rhsTy) && "Unification succeeded, but lhs/rhs types "
          "are different?");
        expr->setType(lhsTy);
        return expr;
      }

      diagnoseInvalidBinaryExpr(expr);
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
      lhsTy = lhsTy->getRValue();
      // Some more sanity checks:
        // Can't have unbound LValues
      assert(lhsTy && "DeclRefExpr has a LValue to an unbound type?");
        // Can't assign to a function
      assert((!lhsTy->is<FunctionType>()) && "Assigning to a function?");

      // Ignore the LValue on the RHS.
      rhsTy = rhsTy->getRValue();

      // Unify
      if(!getSema().unify(lhsTy, rhsTy)) {
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
        diagnoseInvalidBinaryExpr(expr);
        return expr;
      }

      // For ranking comparisons, only allow primitive types except booleans
      // as operands.
      if (expr->isRankingComparison()) {
        // Check if both lhs and rhs obey this condition
        bool lhsOk = (lhsTy->is<PrimitiveType>() && !lhsTy->isBoolType());
        bool rhsOk = (rhsTy->is<PrimitiveType>() && !rhsTy->isBoolType());
        if(!(lhsOk && rhsOk)) {
          diagnoseInvalidBinaryExpr(expr);
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

      // for logical AND and OR operations, only allow booleans
      // as LHS and RHS.
      if (lhsTy->isBoolType() && rhsTy->isBoolType())
        return finalizeBooleanExpr(expr);

      // Else, this is an error.
      diagnoseInvalidBinaryExpr(expr);
      return expr;
    }


    //----------------------------------------------------------------------//
    // Other helper methods
    //----------------------------------------------------------------------//
    // Various helper methods unrelated to semantics
    //----------------------------------------------------------------------//

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

// ExprFinalizer
//  This class walks the Expression tree, simplifying every type.
//  When a type cannot be simplified (due to an inference error)
//  it diagnoses it and replaces the type with ErrorType.
class Sema::ExprFinalizer : ASTWalker {
  using Inherited = TypeVisitor<ExprFinalizer, Type>;
  friend Inherited;

  public:
    Sema& sema;
    ASTContext& ctxt;
    DiagnosticEngine& diags;

    // This is a pointer to the expression which has an ErrorType and has 
    // requested to mute every diagnostic that might be emitted when visiting
    // it's children.
    Expr* diagsMuter = nullptr;

    void muteDiagsForChildren(Expr* expr) {
      diags.setIgnoreAll(true);
      diagsMuter = expr;
    }

    void tryUnmuteDiags(Expr* expr) {
      if(expr == diagsMuter) {
        diags.setIgnoreAll(false);
        diagsMuter = nullptr;
      }
    }

    ExprFinalizer(Sema& sema) : sema(sema), ctxt(sema.getASTContext()),
      diags(sema.getDiagnosticEngine()) {
    }

    ~ExprFinalizer() {
      sema.resetTypeVariables();
    }

    Expr* finalize(Expr* expr) {
      Expr* e = walk(expr);
      assert(e && "expr is null post walk");
      return e;
    }

    std::pair<Expr*, bool> handleExprPre(Expr* expr) {
      Type type = expr->getType();
      assert(type && "Expr has a null type!");

      // Simplify the type.
      type = sema.simplify(type);

      // If the type is nullptr, it means we have an inference error.
      // Set the type to ErrorType and diagnose.
      if (!type) {
        diags.report(DiagID::sema_failed_infer, expr->getRange());
        type = ErrorType::get(ctxt);
        // Mute inference errors for the children.
        muteDiagsForChildren(expr);
      }
      // Inference succeeded, but maybe we have an ErrorType somewhere in
      // there. If that's the case, mute diagnostics for the children exprs.
      else if(type->hasErrorType()) {
        muteDiagsForChildren(expr);
      }
      // Set the type
      expr->setType(type);
      return {expr, true};
    }

    Expr* handleExprPost(Expr* expr) {
      tryUnmuteDiags(expr);
      return expr;
    }
};

Expr* Sema::typecheckExpr(Expr* expr) {
  assert(expr && "null input");
  expr = ExprChecker(*this).check(expr);
  expr = ExprFinalizer(*this).finalize(expr);
  // Success is if the type of the expression isn't ErrorType.
  return expr;
}

bool Sema::typecheckExprOfType(Expr*& expr, Type type) {
  assert(expr && "null input");

  expr = ExprChecker(*this).check(expr);
  bool success = unify(type, expr->getType());
  expr = ExprFinalizer(*this).finalize(expr);

  return success;
}

bool Sema::typecheckCondition(Expr*& expr) {
  expr = ExprChecker(*this).check(expr);
  expr = ExprFinalizer(*this).finalize(expr);
  // ErrorType ? Return false.
  if(expr->getType()->hasErrorType()) return false;
  // Else, return true if we have a numeric or boolean type.
  return expr->getType()->getRValue()->isNumericOrBool();
 }