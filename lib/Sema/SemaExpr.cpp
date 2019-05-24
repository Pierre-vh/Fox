//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : SemaExpr.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  This file implements Sema methods related to Exprs and most of the 
//  expr checking logic.
//----------------------------------------------------------------------------//

#include "Fox/Sema/Sema.hpp"
#include "Fox/AST/Expr.hpp"
#include "Fox/AST/Type.hpp"
#include "Fox/AST/Types.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/Common/LLVM.hpp"
#include "Fox/AST/ASTWalker.hpp"
#include "Fox/AST/TypeVisitor.hpp"
#include "Fox/AST/ASTContext.hpp"
#include "Fox/AST/ASTVisitor.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"
#include <utility>

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
    // "diagnose" methods
    //----------------------------------------------------------------------//
    // Methods designed to diagnose specific situations.
    //----------------------------------------------------------------------//

    // (Note) example: "'foo' declared here with type 'int'"
    void noteIsDeclaredHereWithType(FileID inFile, ValueDecl* decl) {
      assert(inFile && "invalid FileID");
      Identifier id = decl->getIdentifier();
      SourceRange range = decl->getIdentifierRange();
      Type declType = decl->getValueType();

      if(!isWellFormed(declType)) return;

      if (isa<BuiltinFuncDecl>(decl)) {
        diagEngine.report(DiagID::is_a_builtin_func_with_type, inFile)
          .addArg(id).addArg(declType);
        return;
      }

      assert(id && range && "ill formed ValueDecl");
      diagEngine.report(DiagID::declared_here_with_type, range)
        .addArg(id).addArg(declType);
    }

    // (Error) Diagnoses an invalid cast 
    void diagnoseInvalidCast(CastExpr* expr) {
      SourceRange range = expr->getCastTypeLoc().getSourceRange();
      Type childTy = expr->getChild()->getType();
      Type goalTy = expr->getCastTypeLoc().getType();

      if(!isWellFormed({childTy, goalTy})) return;

      diagEngine
        .report(DiagID::invalid_explicit_cast, range)
        .addArg(childTy)
        .addArg(goalTy)
        .setExtraRange(expr->getChild()->getSourceRange());
    }

    // (Warning) Diagnoses a redudant cast (when the cast goal 
    // and the child's type are equal)
    void warnRedundantCastExpr(CastExpr* expr, TypeLoc castTL) {
      Type castTy = castTL.getType();
      if(!isWellFormed(castTy)) return;

      diagEngine
        .report(DiagID::useless_redundant_cast, castTL.getSourceRange())
        .addArg(castTy)
        .setExtraRange(expr->getChild()->getSourceRange());
    }

    void diagnoseHeteroArrLiteral(ArrayLiteralExpr* expr, Expr* faultyElem,
                                  Type supposedType) {
      assert(faultyElem && "no element pointed");
      diagEngine
        // Precise error loc is the first element that failed the inferrence,
        // extended range is the whole arrayliteral's.
        .report(DiagID::unexpected_elem_of_type_in_arrlit, 
                faultyElem->getSourceRange())
          .addArg(faultyElem->getType())
        // Sometimes, the supposed type might contain a type variable.
        // Try to simplify the type to produce a better diagnostic!
        .addArg(sema.trySimplify(supposedType))
        .setExtraRange(expr->getSourceRange());
    }

    void diagnoseInvalidUnaryOpChildType(UnaryExpr* expr) {
      Expr* child = expr->getChild();
      Type childTy = child->getType();

      if(!isWellFormed(childTy)) return;

      diagEngine
        .report(DiagID::unaryop_bad_child_type, expr->getOpRange())
        // Use the child's range as the extra range.
        .setExtraRange(child->getSourceRange())
        .addArg(expr->getOpSign()) // %0 is the operator's sign as text
        .addArg(childTy); // %1 is the type of the child
    }

    void diagnoseInvalidArraySubscript(SubscriptExpr* expr,
                                       SourceRange range, SourceRange extra) {
      Expr* child = expr->getBase();
      Type childTy = child->getType();

      Expr* idxE = expr->getIndex();
      Type idxETy = idxE->getType();

      if(!isWellFormed({childTy, idxETy})) return;

      diagEngine
        .report(DiagID::arrsub_invalid_types, range)
        // %0 is subscripted value's type, %1 is the index's expr type;
        .addArg(childTy)
        .addArg(idxETy)
        .setExtraRange(extra);
    }

    void diagnoseInvalidBinaryExpr(BinaryExpr* expr) {
      SourceRange opRange = expr->getOpRange();
      SourceRange exprRange = expr->getSourceRange();
      Type lhsTy = expr->getLHS()->getType();
      Type rhsTy = expr->getRHS()->getType();

      if(!isWellFormed({lhsTy, rhsTy})) return;

      // Use the specific diagnostic for assignements
      if(expr->isAssignement()) 
        return diagnoseInvalidAssignement(expr, lhsTy, rhsTy);

      diagEngine
        .report(DiagID::binexpr_invalid_operands, opRange)
        .addArg(expr->getOpSign())
        .addArg(lhsTy)
        .addArg(rhsTy)
        .setExtraRange(exprRange);
    }

    // Diagnoses an undeclared identifier
    void diagnoseUndeclaredIdentifier(SourceRange range, Identifier id) {
      diagEngine.report(DiagID::undeclared_id, range).addArg(id);
    }

    // Diagnoses an ambiguous identifier
    void diagnoseAmbiguousIdentifier(SourceRange range, Identifier id,
                                     const LookupResult& results) {
      // First, display the "x" is ambiguous error
      diagEngine.report(DiagID::ambiguous_ref, range).addArg(id);
      // Now, iterate over the lookup results and emit notes
      // for each candidate.
      assert(results.isAmbiguous());
      for(auto result : results) {
        if (auto* builtin = dyn_cast<BuiltinFuncDecl>(result)) {
          diagEngine
            .report(DiagID::potential_candidate_is_builtin, range.getFileID())
            .addArg(builtin->getIdentifier())
            .addArg(builtin->getValueType());

        }
        else {
          diagEngine.report(DiagID::potential_candidate_here, 
            result->getIdentifierRange());
        }
      }
    }

    // Diagnoses an unassignable expression
    void diagnoseUnassignableExpr(BinaryExpr* expr) {
      assert(expr->isAssignement());
      if(!isWellFormed(expr->getLHS()->getType())) return;

      SourceRange lhsRange = expr->getLHS()->getSourceRange();
      SourceRange opRange = expr->getOpRange();
      diagEngine.report(DiagID::unassignable_expr, lhsRange)
        .setExtraRange(opRange);
    }

    // Diagnoses an illegal assignement.
    void diagnoseInvalidAssignement(BinaryExpr* expr, Type lhsTy, Type rhsTy) {
      assert(expr->isAssignement());
      if(!isWellFormed({lhsTy, rhsTy})) return;

      SourceRange lhsRange = expr->getLHS()->getSourceRange();
      SourceRange rhsRange = expr->getRHS()->getSourceRange();

      // Diag is (roughly) "can't assign a value of type (lhs) to type (rhs)
      diagEngine.report(DiagID::invalid_assignement, rhsRange)
        .setExtraRange(lhsRange)
        .addArg(rhsTy).addArg(lhsTy);
    }

    // Diagnoses a variable being used inside it's own initial value.
    void diagnoseVarInitSelfRef(VarDecl* decl, 
      UnresolvedDeclRefExpr* udre) {
      SourceRange range = udre->getSourceRange();
      SourceRange extra = decl->getIdentifierRange();
      diagEngine.report(DiagID::var_init_self_ref, range)
        .setExtraRange(extra);
    }

    // Diagnoses an expression that isn't a function but was
    // expected to be one.
    void diagnoseExprIsNotAFunction(Expr* callee) {
      SourceRange range = callee->getSourceRange();
      Type ty = callee->getType();

      if(!isWellFormed(ty)) return;

      diagEngine.report(DiagID::expr_isnt_func, range)
        .addArg(ty);
    }

    // Diagnoses a bad function call where the number of arguments
    // provided didn't match the number of parameters expected.
    void diagnoseArgcMismatch(CallExpr* call, std::size_t argsProvided, 
      std::size_t argsExpected) {
      assert(argsProvided != argsExpected);

      Expr* callee = call->getCallee();
      std::string calleePrettyName = getCalleePrettyName(callee);

      // For now, only a DeclRefExpr can have a FunctionType. But if that
      // changes in the future, remove this assert and add alternative diags.
      assert(callee && "callee isn't a DeclRefExpr");

      DiagID diag;
      // Use the most appropriate diagnostic based on the situation
      if(argsProvided == 0) 
        diag = DiagID::cannot_call_with_no_args;
      else if(argsProvided < argsExpected) 
        diag = DiagID::not_enough_args_in_func_call;
      else 
        diag = DiagID::too_many_args_in_func_call;

      // Report the diagnostic
      diagEngine.report(diag, callee->getSourceRange())
        .addArg(calleePrettyName);

      // If the callee is a DeclRefExpr, emit a note to point at the
      // decl it references.
      if(auto declref = dyn_cast<DeclRefExpr>(callee))
        noteIsDeclaredHereWithType(call->getBeginLoc().getFileID(), 
                                   declref->getDecl());
    }

    // Diagnoses a bad function call where the types of the arguments
    // didn't match
    void diagnoseBadFunctionCall(CallExpr* call) {
      assert((call->getCallee() != nullptr) && "no callee");
      assert(call->numArgs() && "numArgs cannot be zero!");

      Expr* callee = call->getCallee();
      std::string calleePrettyName = getCalleePrettyName(callee);

      // Retrieve a user-friendly presentation of the args
      std::string argsAsStr = getArgsAsString(call);

      diagEngine.report(DiagID::cannot_call_func_with_args, call->getArgsRange())
        .addArg(calleePrettyName)
        .addArg(argsAsStr)
        .setExtraRange(callee->getSourceRange());

      // If the callee is a DeclRefExpr, emit a note to point at the
      // decl it references.
      if(auto declref = dyn_cast<DeclRefExpr>(callee))
        noteIsDeclaredHereWithType(call->getBeginLoc().getFileID(), 
                                   declref->getDecl());
    }

    // Diagnoses the presence of a function type in an array literal.
    void diagnoseFunctionTypeInArrayLiteral(ArrayLiteralExpr* lit, Expr* fn) {
      assert(fn->getType()->getRValue()->is<FunctionType>() && "wrong func");
      diagEngine.report(DiagID::func_type_in_arrlit, fn->getSourceRange())
        // Maybe displaying the whole array is too much? I think it's great
        // because it gives some context, but maybe I'm wrong.
        .setExtraRange(lit->getSourceRange());
    }

    // Diagnoses an attempt to access an unknown member of a type.
    // e.g. "string".foobar (foobar doesn't exist)
    void diagnoseUnknownTypeMember(UnresolvedDotExpr* expr) {
      Expr* base = expr->getBase();
      SourceRange baseRange = base->getSourceRange();
      Type baseType = base->getType();
      Identifier membID = expr->getMemberIdentifier();
      SourceRange memberRange = expr->getMemberIdentifierRange();

      if(!isWellFormed(baseType)) return;

      diagEngine.report(DiagID::type_has_no_member_named, memberRange)
        .addArg(baseType)
        .addArg(membID)
        .setExtraRange(baseRange);
    }

    //----------------------------------------------------------------------//
    // Finalize methods
    //----------------------------------------------------------------------//
    // There is no set rule on what a finalize() method can do, but generally 
    // they're used when the expression is known to be correct and we just
    // need to "finalize" it: guess its type, resolve it, replace it, etc.
    //
    // TL;DR: These methods always assume that the Expr is semantically correct.
    //----------------------------------------------------------------------//

    /// Finalizes a boolean expression (e.g. conditional/logical
    /// expressions such as LAnd, LNot, LE, GT, etc..)
    Expr* finalizeBooleanExpr(Expr* expr) {
      expr->setType(BoolType::get(ctxt));
      return expr;
    }

    Expr* finalizeCastExpr(CastExpr* expr, bool isRedundant) {
      TypeLoc castTL = expr->getCastTypeLoc();

      if (isRedundant) { 
        expr->markAsUselesss();
        warnRedundantCastExpr(expr, castTL);
      }

      expr->setType(castTL.getType());
      return expr;
    }

    Expr* finalizeEmptyArrayLiteral(ArrayLiteralExpr* expr) {
      assert((expr->numElems() == 0) && "Only for empty Array Literals");
      // For empty array literals, the type is going to be a fresh
      // TypeVariable inside an Array. e.g. [$T0]
      Type type = sema.createNewTypeVariable();
      type = ArrayType::get(ctxt, type); 
      expr->setType(type);
      return expr;
    }

    Expr* finalizeConcatBinaryExpr(BinaryExpr* expr) {
      // For concatenation, the type is always string.
      // We'll also change the add operator to become the concat operator.
      expr->setType(StringType::get(ctxt));
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
          // and these assertions are triggered in valid code, replace them
          // by conditions.
          assert(var->getInitExpr() && var->getInitExpr()->getSourceRange()
                  .contains(udre->getSourceRange()));
          diagnoseVarInitSelfRef(var, udre);
          // This is an error, so return an ErrorExpr
          return ErrorExpr::create(ctxt, udre);
        }
      }

      // If we're referring to a ParamDecl, mark it as being used.
      if(ParamDecl* param = dyn_cast<ParamDecl>(found))
        param->setIsUsed();

      // Create the resolved DeclRef.
      DeclRefExpr* resolved = 
        DeclRefExpr::create(ctxt, found, udre->getSourceRange());
      
      Type valueType = found->getValueType();
      assert(valueType && "ValueDecl doesn't have a Type!");
      // If it's a non const ValueDecl, wrap it in a LValue
      if(!found->isConst()) {
        assert(!isa<FuncDecl>(found) && "FuncDecl are always const!");
        valueType = LValueType::get(ctxt, valueType);
      }

      resolved->setType(valueType);
      return resolved;
    }

    //----------------------------------------------------------------------//
    // ASTWalker overrides
    //----------------------------------------------------------------------//

    virtual Expr* handleExprPost(Expr* expr) {
      assert(expr && "Expr cannot be null!");
      expr = visit(expr);
      assert(expr && "Expr cannot be null!");
      // Check if the expr is typed. If it isn't, that means typechecking 
      // failed for this node, so set its type to ErrorType.
      if (!expr->getType())
        expr->setType(ErrorType::get(ctxt));
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
    // Typechecks single Expr
    //
    // NOTE: No "visit" method present here will manually visit the children
    // of the expr. This is always done by the ASTWalker before calling
    // the "visit" methods. (Visitation is done in postorder)
    //----------------------------------------------------------------------//

    Expr* visitErrorExpr(ErrorExpr*) {
      // ErrorExprs shouldn't be visited again.
      fox_unreachable("Expression visited twice");
    }

    Expr* visitBinaryExpr(BinaryExpr* expr) {
      using BOp = BinaryExpr::OpKind;

      assert(expr->isValidOp() && "BinaryExpr with Invalid Op found");

      // Fetch the types of the LHS and RHS 
      Type lhsTy = expr->getLHS()->getType();
      Type rhsTy = expr->getRHS()->getType();
      assert(lhsTy && rhsTy && "untyped exprs");

      // Check that the types are well formed. If they aren't, don't
      // bother typechecking the expr.
      if (!isWellFormed({lhsTy, rhsTy})) return expr;

      // Handle assignements early: let checkAssignementBinaryExpr do it.
      if (expr->isAssignement())
        return checkAssignementBinaryExpr(expr, lhsTy, rhsTy);

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
        case BOp::Pow:
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
      // Get the types
      Type childTy = expr->getChild()->getType();
      Type goalTy = expr->getCastTypeLoc().getType();

      // Check that the types are well formed. If they aren't, don't
      // bother typechecking the expr.
      if (!isWellFormed({childTy, goalTy})) return expr;

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

      // Try unification with the custom comparator.
      if (sema.unify(childTy, goalTy, comparator))
        return finalizeCastExpr(expr, isPerfectEquality);      

      diagnoseInvalidCast(expr);
      return expr;
    }

    Expr* visitUnaryExpr(UnaryExpr* expr) {
      using UOp = UnaryExpr::OpKind;
      Type childTy = expr->getChild()->getType();

      // Check that the child's type is well formed. If it isn't, don't
      // bother typechecking the expr.
      if (!isWellFormed(childTy)) return expr;

      switch (expr->getOp()) {
        case UOp::Invalid:
          fox_unreachable("UnaryExpr with Invalid Op found");
        // LNot '!' operator: Only applicable on booleans.
        case UOp::LNot:
          if(childTy->isBoolType())
            return finalizeBooleanExpr(expr);
          break;
        // Unary Plus '+' and Minus '-' operators: only applicable
        // on numeric types.
        case UOp::Minus:
        case UOp::Plus:
          if (childTy->isNumericType()) {
            // The type of the expr is the same as its child (without
            // LValue if present)
            expr->setType(childTy->getRValue());
            return expr;
          }
          break;
        case UOp::ToString:
          return checkToStringUnaryExpr(expr);
        default:
          fox_unreachable("Unhandled Unary Op kind");
      }

      // If we get here, the expression is invalid.
      diagnoseInvalidUnaryOpChildType(expr);
      return expr;
    }

    Expr* visitSubscriptExpr(SubscriptExpr* expr) {
      // Fetch the base
      Expr* base = expr->getBase();
      Type baseTy = base->getType();
      // Fetch the index
      Expr* idx = expr->getIndex();
      Type idxTy = idx->getType();

      // Check that the types are well formed. If they aren't, don't
      // bother typechecking the expr.
      if (!isWellFormed({idxTy, baseTy})) return expr;

      bool canLValue = true;
      Type subscriptType;

      // Check if the base is an Array Type, ignoring LValues 
      // if present.
      if (auto arr  = baseTy->getRValue()->getAs<ArrayType>()) {
        subscriptType = arr->getElementType();
        assert(subscriptType && "ArrayType had no element type!");
      }
      // String types are also allowed, but they cannot be mutated.
      // (-> It's just a shorthand for a getchar-like function)
      else if (baseTy->isStringType()) {
        subscriptType = CharType::get(ctxt);
        canLValue = false;
      }
      // if it's neither, we can't subscript.
      else {
			  diagnoseInvalidArraySubscript(expr, base->getSourceRange(), 
                                      idx->getSourceRange());
        return expr;
      }

      // Idx type must be int.
      if (!idxTy->isIntType()) {
        // Diagnose with the primary range being the idx's range
			  diagnoseInvalidArraySubscript(expr, idx->getSourceRange(), 
                                      base->getSourceRange());
        return expr;
      }
      assert(subscriptType);

      // Check if the subscript's type should be an LValue
      if(canLValue && isAccessOnExprAssignable(base))
        subscriptType = LValueType::get(ctxt, subscriptType);
      expr->setType(subscriptType);
      return expr;
    }

    Expr* visitUnresolvedDotExpr(UnresolvedDotExpr* expr) {
      // Currently there's only builtin types, no user-defined ones, 
      // so we know the base's type is always a builtin one.
      // This means that we can directly call resolveBuiltinTypeMember to
      // try to resolve this expr.
      BuiltinMemberRefExpr* resolved = sema.resolveBuiltinTypeMember(expr);
      if (resolved) return resolved;

      // Failed resolution : diagnose & return ErrorExpr.
      diagnoseUnknownTypeMember(expr);
      return ErrorExpr::create(ctxt, expr); 
    }

    Expr* visitBuiltinMemberRefExpr(BuiltinMemberRefExpr*) {
      // Shouldn't happen at all.
      fox_unreachable("Expr checked twice!");
    }

    Expr* visitUnresolvedDeclRefExpr(UnresolvedDeclRefExpr* expr) {
      Identifier id = expr->getIdentifier();
      SourceRange range = expr->getSourceRange();

      LookupResult results;
      sema.doUnqualifiedLookup(results, id, expr->getBeginLoc());

      // No results -> undeclared identifier
      if(results.isEmpty()) {
        diagnoseUndeclaredIdentifier(range, id);
        // Return an ErrorExpr on error.
        return ErrorExpr::create(ctxt, expr);
      }

      // Ambiguous result
      if(results.isAmbiguous()) {
        // Try to remove illegal redecls from the results
        if (!removeIllegalRedecls(results)) {
          diagnoseAmbiguousIdentifier(range, id, results);
          // Return an ErrorExpr on error.
          return ErrorExpr::create(ctxt, expr);
        }
      }

      // Correct, unambiguous result.
      NamedDecl* decl = results.getIfSingleResult();
      assert(decl 
        && "not ambiguous, not empty, but doens't contain a single result?");

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

      // If the callee's type isn't well formed, don't bother
      // typechecking the expr.
      if(!isWellFormed(calleeTy)) return expr;
      
      if(!calleeTy->is<FunctionType>()) {
        diagnoseExprIsNotAFunction(callee);
        return expr;
      }

      // If the callee is a BuiltinMemberRefExpr, mark it as being
      // called. This is done early because it must be set
      // even when the call isn't valid to avoid useless diagnostics
      // being emitted during expression finalization.
      if(auto bmr = dyn_cast<BuiltinMemberRefExpr>(callee))
        bmr->setIsCalled(true);

      FunctionType* fnTy = calleeTy->castTo<FunctionType>();

      // Check arg count
      std::size_t callArgc = expr->numArgs();
      std::size_t expectedArgc = fnTy->numParams();
      if(callArgc != expectedArgc) {
        diagnoseArgcMismatch(expr, callArgc, expectedArgc);
        return expr;
      }

      // Check that the function is callable with these arguments.
      if(fnTy->numParams()) {
        bool hasArgTypeMismatch = false;
        // Check arg types
        for(std::size_t idx = 0; idx < callArgc; idx++) {
          Type paramType = fnTy->getParam(idx);
          Expr* arg = expr->getArg(idx);
          Type argType = arg->getType();

          // If the argument's type isn't well formed, abort.
          if(!isWellFormed(argType)) return expr;

          assert(paramType && argType && "types cant be nullptrs!");
          // Check that the types match.
          if (!sema.unify(paramType, argType)) {
            hasArgTypeMismatch = true;
            continue;
          }
        }

        if (hasArgTypeMismatch) {
          diagnoseBadFunctionCall(expr);
          return expr;
        }
      }

      // Call should be ok. The type of the CallExpr is the return type
      // of the function.
      Type ret = fnTy->getReturnType();
      assert(ret && "function return type is null");
      expr->setType(ret);
      return expr;
    }
      
    // Trivial literals: the expr's type is simply the corresponding
    // type. Int for a Int literal, etc.
    Expr* visitCharLiteralExpr(CharLiteralExpr* expr) {
      expr->setType(CharType::get(ctxt));
      return expr;
    }

    Expr* visitIntegerLiteralExpr(IntegerLiteralExpr* expr) {
      expr->setType(IntegerType::get(ctxt));
      return expr;
    }

    Expr* visitDoubleLiteralExpr(DoubleLiteralExpr* expr) {
      expr->setType(DoubleType::get(ctxt));
      return expr;
    }

    Expr* visitBoolLiteralExpr(BoolLiteralExpr* expr) {
      expr->setType(BoolType::get(ctxt));
      return expr;
    }

    Expr* visitStringLiteralExpr(StringLiteralExpr* expr) {
      expr->setType(StringType::get(ctxt));
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


    /// \returns true if the result of an "access" operation on \p base should
    ///               be assignable.
    /// "Access" here means both susbcripting and member access.
    /// This will return true in 2 cases:
    ///   - if \p base's type is an LValue
    ///   - if \p base is a reference to a declaration (let or not)
    bool isAccessOnExprAssignable(Expr* base) {
      Type type = base->getType();
      // Subscript is automatically assignable if the base is assignable
      if(type->isAssignable())
        return true;
      // Else, it's only assignable if the Expr is a DeclRefExpr.
      // FIXME: If I add ParenExpr, take them into account here (by
      //        ignoring them)
      // FIXME: Can this be done better?
      return isa<DeclRefExpr>(base);
    }

    /// Checks if an expression can legally appear inside an array literal.
    /// If it can't and it can be diagnosed, diagnoses it.
    bool checkIfLegalWithinArrayLiteral(ArrayLiteralExpr* lit, Expr* expr) {
      Type ty = expr->getType()->getRValue();
      // Functions aren't first-class yet, so we can't allow
      // function types inside array literals.
      if(ty->is<FunctionType>()) {
        diagnoseFunctionTypeInArrayLiteral(lit, expr);
        return false;
      }
      // We also forbid ill-formed types.
      return isWellFormed(ty);
    }

    /// Typechecks a non empty array literal and deduces its type.
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

        // Retrieve the type of the element, ignoring LValues
        // if present.
        Type elemTy = elem->getType()->getRValue();

        // First loop, set the proposed type and continue.
        if (!proposedType) {
          proposedType = elemTy;
          continue;
        }

        // Next iterations: Unify the element's type with the proposed type.
        if (!sema.unify(proposedType, elemTy)) {
          diagnoseHeteroArrLiteral(expr, elem, proposedType);
          continue;
        }
      }

      // Set the type to an ArrayType of the
      // type if the expr is still considered valid.
      if(isValid)
        expr->setType(ArrayType::get(ctxt, proposedType));
      return expr;
    }

    // Typecheck a basic binary expression that involves numeric types. 
    // This includes multiplicative/additive/exponent
    // operations (except concatenation).

    Expr*
    checkBasicNumericBinaryExpr(BinaryExpr* expr, Type lhsTy, Type rhsTy) {
      assert((expr->isAdditive() 
            || expr->isPower() 
            || expr->isMultiplicative()) && "wrong function!");
        
      // Check that lhs and rhs unify and that they're both numeric
      // types.
      if(sema.unify(lhsTy, rhsTy) && 
        (lhsTy->isNumericType() && rhsTy->isNumericType())) {
        expr->setType(lhsTy);
        return expr;
      }

      diagnoseInvalidBinaryExpr(expr);
      return expr;
    }

    // Returns true if this combination of operator/types
    // is eligible to be a concatenation operation
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
    Expr* checkAssignementBinaryExpr(BinaryExpr* expr, Type lhsTy, Type rhsTy) {
      assert(expr->isAssignement() && "wrong function!");
      
      if(!lhsTy->isAssignable()) {
        diagnoseUnassignableExpr(expr);
        return expr;
      }

      // Can't assign to a function, because DeclRefs to functions
      // will always generate RValues, so we shouldn't even get here.
      assert((!lhsTy->is<FunctionType>()) && "Assigning to a function?");

      // Unify
      if(!sema.unify(lhsTy, rhsTy)) {
        // Type mismatch
        diagnoseInvalidAssignement(expr, lhsTy, rhsTy);
        return expr;
      }

      // Everything's fine, the type of the expr is the type of its RHS
      // without LValues if present.
      expr->setType(rhsTy->getRValue());
      return expr;
    }

    // Typechecks a comparative operation
    //  \param lhsTy The type of the LHS (must not be null)
    //  \param rhsTy The type of the RHS (must not be null)
    Expr* 
    checkComparisonBinaryExpr(BinaryExpr* expr, Type lhsTy, Type rhsTy) {
      assert(expr->isComparison() && "wrong function!");

      if (!sema.unify(lhsTy, rhsTy)) {
        diagnoseInvalidBinaryExpr(expr);
        return expr;
      }

      // For ranking comparisons, only allow primitive types except booleans
      // as operands.
      if (expr->isRankingComparison()) {
        bool lhsOk = (lhsTy->isPrimitiveType() && !lhsTy->isBoolType());
        bool rhsOk = (rhsTy->isPrimitiveType() && !rhsTy->isBoolType());
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

    // Typechecks a use of the '$' operator, the toString operator.
    Expr* checkToStringUnaryExpr(UnaryExpr* expr) {
      assert((expr->getOp() == UnaryExpr::OpKind::ToString) &&
        "wrong function");
      Type childTy = expr->getChild()->getType();
      Type stringType = StringType::get(ctxt);

      // We only allow non-void primitive types as the child's type.
      if (!childTy->isPrimitiveType() || childTy->isVoidType()) {
        diagnoseInvalidUnaryOpChildType(expr);
        return expr;
      }

      // Check that the child isn't already a string type 
      // (warn the user if that's the case)
      if (childTy->isStringType()) {
        diagEngine.report(DiagID::useless_redundant_cast, expr->getOpRange())
          .addArg(stringType)
          .setExtraRange(expr->getChild()->getSourceRange());
      }

      expr->setType(stringType);
      return expr;
    }

    //----------------------------------------------------------------------//
    // Other helper methods
    //----------------------------------------------------------------------//
    // Various helper methods unrelated to semantics
    //----------------------------------------------------------------------//

    // Removes illegal redeclarations from an ambigous lookup result.
    // Returns true if the new LookupResult is now unambiguous.
    bool removeIllegalRedecls(LookupResult& result) {
      assert(result.isAmbiguous() && "only ambiguous lookup results allowed");
      // FIXME: Can this be made more efficient?
      SmallVector<NamedDecl*, 4> newResults;
      for (NamedDecl* decl : result.getDecls()) {
        if(!decl->isIllegalRedecl())
          newResults.push_back(decl);
      }
      result.getDecls() = newResults;
      return !result.isAmbiguous();
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

    /// \returns the pretty name of a callee.
    ///     For DeclRefExprs, returns the name of the declaration it
    ///     references.
    ///     For BuiltinMemberRefExpr, returns type.member
    std::string getCalleePrettyName(Expr* expr) {
      if(auto declref = dyn_cast<DeclRefExpr>(expr))
        return declref->getDecl()->getIdentifier().getStr().to_string();
      if (auto builtinMemb = dyn_cast<BuiltinMemberRefExpr>(expr)) {
        std::stringstream ss;
        ss << builtinMemb->getBase()->getType()
           << '.' 
           << builtinMemb->getMemberIdentifier();
        return ss.str();
      }
      fox_unreachable("unknown callee kind");
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
    // requested to mute every diagnostic pertaining to inference issues for
    // its children.
    Expr* inferenceDiagMuter = nullptr;
    bool canEmitInferenceDiagnostics = true;

    void muteInferenceErrors(Expr* expr) {
      canEmitInferenceDiagnostics = false;
      inferenceDiagMuter = expr;
    }

    void unmuteInferenceErrors(Expr* expr) {
      if(expr == inferenceDiagMuter) {
        canEmitInferenceDiagnostics = true;
        inferenceDiagMuter = nullptr;
      }
    }

    ExprFinalizer(Sema& sema) : sema(sema), ctxt(sema.ctxt),
      diags(sema.diagEngine) {
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
        if(canEmitInferenceDiagnostics)
          diags.report(DiagID::expr_failed_infer, expr->getSourceRange());
        type = ErrorType::get(ctxt);
        // Mute inference errors for the children.
        muteInferenceErrors(expr);
      }
      // Inference succeeded, but maybe we have an ErrorType somewhere in
      // there. If that's the case, mute diagnostics for the children exprs.
      else if(type->hasErrorType()) {
        muteInferenceErrors(expr);
      }
      // Set the type
      expr->setType(type);
      return {expr, true};
    }

    Expr* handleExprPost(Expr* expr) {
      unmuteInferenceErrors(expr);

      // If the Expr is a BuiltinMemberRefExpr and it's not called, it's
      // an error.
      if (auto bmr = dyn_cast<BuiltinMemberRefExpr>(expr)) {
        if (!bmr->isCalled())
          diags.report(DiagID::uncalled_bound_member_function, 
                       bmr->getSourceRange());
      }

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
  return expr->getType()->isNumericOrBool();
 }