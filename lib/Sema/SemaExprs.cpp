//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : SemaExprs.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  This file implements Sema methods related to Exprs
//----------------------------------------------------------------------------//

// Short to-do list (to do in order)
/*
    Re-order methods in the ExprChecker. Document them better, write "category" 
    headers, etc.
    
    Write LIT tests for UnaryExpr & CastExpr 
    (take advantage of the redudant cast warning)

    Check again how verifymode works with -werror (should I use expect-warning or expect-error?)
      -> Maybe let the DV allow an error with expect-warn?

    Move on
*/

#include "Fox/Sema/Sema.hpp"
#include "Fox/AST/Expr.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/Common/LLVM.hpp"
#include "Fox/AST/ASTWalker.hpp"
#include "Fox/AST/ASTVisitor.hpp"

#include <utility>
#include <iostream>

using namespace fox;

namespace {
  // Various helper functions

  // If "type" is a CellType* with a substitution, returns the substitution,
  // else, returns "type".
  Type defer_if(Type type) {
    if (auto* ptr = type->getAs<CellType>()) {
      // if the type has a substitution, return it, else
      // just return the argument.
      if (Type sub = ptr->getSubstitution())
        return sub;
    }
    return type;
  }

  // Expression checker: Classic visitor, the visitXXX functions
  // all check a single node. They do not orchestrate visitation of
  // the children, because that is done in the ASTWalker
  //
  // Every visitation method return a pointer to an Expr*, which is the current 
  // expr
  // OR the expr that should take it's place. This can NEVER be null.
  class ExprChecker : public ExprVisitor<ExprChecker, Expr*>, public ASTWalker {
    using Inherited = ExprVisitor<ExprChecker, Expr*>;
    Sema& sema_;
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

      //----------------------------------------------------------------------//
      // Finalize methods
      //----------------------------------------------------------------------//
      // The finalize family of methods will... finalize the given expr.
      // Finalizing can mean a lot of things. Some exprs might want a finalize()
      // method while others won't need one.
      //
      // Usually, in finalizeXXXExpr, we'll set it's type, maybe emit some last
      // minute diagnostic about it, etc. 
      //----------------------------------------------------------------------//

      // Finalizes a valid CastExpr
      Expr* finalizeCastExpr(CastExpr* expr, bool isRedundant) {
        if (isRedundant) {
          // Diagnose the redundant cast
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

      // Finalizes a valid UnaryExpr
      // \param childTy The type of the child as a PrimitiveType.
      Expr* finalizeUnaryExpr(UnaryExpr* expr, PrimitiveType* childTy) {
        assert(childTy && "cannot be nullptr");
        using OP = UnaryExpr::OpKind;
        switch (expr->getOp()) {
          // Logical NOT operator : '!'
          case OP::LNot:
            // Always boolean
            expr->setType(PrimitiveType::getBool(getCtxt()));
            break;
          // Unary Plus '+' and Minus '-'
          case OP::Minus:
          case OP::Plus:
            // Always int or float, never bool, so uprank
            // if boolean.
            if(childTy->isBoolType())
              childTy = PrimitiveType::getInt(getCtxt());
            expr->setType(childTy);
            break;
          case OP::Invalid:
            fox_unreachable("Invalid Unary Operator");
          default:
            fox_unreachable("All cases handled");
        }
        return expr;
      }

      // Finalizes an empty ArrayLiteral
      Expr* finalizeEmptyArrayLiteral(ArrayLiteralExpr* expr) {
        assert((expr->getSize() == 0) && "Only for empty ArrLits");
        // For empty array literals, the type is going to be a fresh
        // celltype inside an Array : Array(CellType(null))
        Type type = CellType::create(getCtxt());
        type = ArrayType::get(getCtxt(), type); 
        expr->setType(type);
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

      virtual Stmt* handleStmtPost(Stmt*) {
        fox_unreachable("Illegal node kind");
      }

      virtual std::pair<Decl*, bool> handleDeclPre(Decl*) {
        fox_unreachable("Illegal node kind");
      }

      virtual Decl* handleDeclPost(Decl*) {
        fox_unreachable("Illegal node kind");
      }


      //----------------------------------------------------------------------//
      // "visit" methods
      //----------------------------------------------------------------------//
      // Theses visit() methods will perform the necessary tasks to check a
      // given expr.
      //----------------------------------------------------------------------//

      Expr* visitBinaryExpr(BinaryExpr*) {
        // Note:
          // Handle arithmetic & text addition
          // Disallow array operation unless *
        fox_unimplemented_feature("BinaryExpr TypeChecking");
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
        assert(getSema().isBound(goalTy) &&
          "Unbound types cannot be present as cast goals!");

        // Check for Error Types. If one of the types is an ErrorType
        // just abort.
        if (childTy->is<ErrorType>() && goalTy->is<ErrorType>())
          return expr;

        // Casting to a String  
          // Check that the child's type is a primitive type.
        if (goalTy->isStringType() && Sema::isBound(childTy)) {
          // If the expr's type isn't a primitive type, diagnose
          // the invalid cast.
          if (!childTy->is<PrimitiveType>()) 
            diagnoseInvalidCast(expr);
          
          return finalizeCastExpr(expr, childTy->isStringType());
        }
        
        // Casting to anything else
          // For other type of casts, unification is enough to determine
          // if the cast is valid. If unification fails, diagnose + errorType
        if (getSema().unify(childTy, goalTy))
          return finalizeCastExpr(expr, (childTy == goalTy));

        diagnoseInvalidCast(expr);
        return expr;
      }

      Expr* visitUnaryExpr(UnaryExpr* expr) {
        Expr* child = expr->getExpr();
        Type childTy = child->getType();
        // ignore LValue + deref
        childTy = Sema::deref(childTy->ignoreLValue());

        // For any unary operators, we only allow integral types,
        // so check that first.
        if (!Sema::isIntegral(childTy)) {
          // Not an integral type -> error.
          // Emit diag if childTy isn't a ErrorType too
          if (!childTy->is<ErrorType>()) {
            getDiags()
              .report(DiagID::sema_unaryop_bad_child_type, expr->getOpRange())
              // Use the child's range as the extra range.
              .setExtraRange(child->getRange()) 
              .addArg(expr->getOpSign()) // %0 is the operator's sign as text
              .addArg(childTy); // %1 is the type of the child
          }
          return expr;
        }
        
        // If isIntegral returns true, we can safely assume that childTy is a
        // PrimitiveType instance
        PrimitiveType* primChildTy = childTy->castTo<PrimitiveType>();
        return finalizeUnaryExpr(expr, primChildTy);
      }

      Expr* visitArrayAccessExpr(ArrayAccessExpr*) {
        // Note:
          // Check that base is of ArrayType and idx expr
          // is arithmetic and not float
        fox_unimplemented_feature("ArrayAccessExpr TypeChecking");
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

      // Array literals
      Expr* visitArrayLiteralExpr(ArrayLiteralExpr* expr) {
        if (expr->getSize() != 0) {
          Type deduced = deduceTypeOfArrayLiteral(expr);
          if(deduced)
            expr->setType(deduced);
          return expr;
        }
        // Type needs inference
        else
          return finalizeEmptyArrayLiteral(expr);
      }

      //----------------------------------------------------------------------//
      // Helper checking methods
      //----------------------------------------------------------------------//
      // Various semantics-related helper methods 
      //----------------------------------------------------------------------//

      // Deduces the type of a non empty Array literal
      // Returns the type of the literal or nullptr if it can't be calculated.
      Type deduceTypeOfArrayLiteral(ArrayLiteralExpr* expr) {
        assert(expr->getSize() && "Size must be >0");

        // The bound type proposed by unifying the other concrete/bound
        // types inside the array.
        Type boundTy;

        // The type used by unbounds elemTy
        Type unboundTy;

        // Loop over each expression in the literal
        for (auto& elem : expr->getExprs()) {
          // Get the elemTy
          Type elemTy = elem->getType();
          assert(elemTy && "Type cannot be null!");

          // Handle error elem type: we stop here if we have one.
          if (elemTy->is<ErrorType>())
            return nullptr;

          // Special logic for unbound types
          if (!Sema::isBound(elemTy)) {
            // Set unboundTy & continue for first loop
            if (!unboundTy)
              unboundTy = elemTy;
            // Attempt unification
            else if (!getSema().unify(unboundTy, elemTy)) {
              diagnoseHeteroArrLiteral(expr, elem);
              return nullptr;
            }
            continue;
          }

          // From this point, ElemTy is guaranteed to be a bound/concrete type

          // First loop, set boundTy & continue.
          if (!boundTy) {
            boundTy = elemTy;
            continue;
          }

          // Unify elemTy with the bound proposed type.
          if (!getSema().unify(boundTy, elemTy)) {
            diagnoseHeteroArrLiteral(expr, elem);
            return nullptr;
          }

          // Get the highest ranking type of elemTy and boundTy
          boundTy = Sema::getHighestRankedTy(elemTy, boundTy);
          assert(boundTy &&
                 "Couldn't determine the highest ranked type "
                 "but unification succeeded?");
        }
        Type proper;
        // Check if we have an unboundTy and/or a boundTy
        if (unboundTy && boundTy) {
          if (!getSema().unify(unboundTy, boundTy)) {
            // FIXME: Proper diagnosis might be needed here.
            diagnoseHeteroArrLiteral(expr, nullptr);
            return nullptr;
          }
          proper = boundTy; 
        }
        else if (boundTy) proper = boundTy;
        else if (unboundTy) proper = unboundTy;
        else fox_unreachable("Should have at least a boundTy or unboundTy set");
        assert(proper && "the proper type shouldn't be null at this stage");

        // The type of the expr is an array of the proposed type.
        return ArrayType::get(getCtxt(), proper.getPtr());
      }
  };

  // ExprFinalizer, which rebuilds types to remove
  // SemaTypes.
  // Visit methods return pointers to TypeBase. They return nullptr
  // if the finalization failed for this expr.
  // It's still a primitive, test version for now.
  class ExprFinalizer : public TypeVisitor<ExprFinalizer, Type>,
    public ASTWalker {
    ASTContext& ctxt_;
    DiagnosticEngine& diags_;

    public:
      ExprFinalizer(ASTContext& ctxt, DiagnosticEngine& diags) :
        ctxt_(ctxt), diags_(diags) {

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

      Type visitPrimitiveType(PrimitiveType* type) {
        return type;
      }

      Type visitArrayType(ArrayType* type) {
        if (Type elem = visit(type->getElementType())) {
          // Rebuild if needed
          if (elem != type->getElementType())
            return ArrayType::get(ctxt_, elem);
          return type;
        }
        return nullptr;
      }

      Type visitLValueType(LValueType* type) {
        if (Type elem = visit(type->getType())) {
          if (elem != type->getType())
            return LValueType::get(ctxt_, elem);
          return type;
        }
        return nullptr;
      }

      Type visitCellType(CellType* type) {
        if (Type sub = type->getSubstitution())
          return visit(sub);
        return nullptr;
      }

      Type visitErrorType(ErrorType* type) {
        // Error should have been handled already. Don't do anything here.
        return type;
      }
  };
} // End anonymous namespace

Expr* Sema::typecheckExpr(Expr* expr) {
  expr = ExprChecker(*this).walk(expr);
  expr = ExprFinalizer(ctxt_, diags_).walk(expr);
  return expr;
}
