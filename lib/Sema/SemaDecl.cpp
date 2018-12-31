//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : SemaDecl.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  This file implements Sema methods related to Decls and most of the 
//  decl checking logic.
//----------------------------------------------------------------------------//

#include "Fox/Sema/Sema.hpp"
#include "Fox/AST/ASTVisitor.hpp"
#include "Fox/AST/ASTWalker.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"
#include <map>

using namespace fox;


class Sema::DeclChecker : Checker, DeclVisitor<DeclChecker, void> {
  using Inherited = DeclVisitor<DeclChecker, void>;
  friend Inherited;
  public:
    DeclChecker(Sema& sema) : Checker(sema) {}

    void check(Decl* decl) {
      assert(decl);
      assert(decl->isUnchecked() && "Decl has already been checked!");
      visit(decl);
    }

  private:
    //----------------------------------------------------------------------//
    // Diagnostic methods
    //----------------------------------------------------------------------//
    // The diagnose family of methods are designed to print the most relevant
    // diagnostics for a given situation.
    //----------------------------------------------------------------------//

    // Diagnoses an illegal variable redeclaration. 
    // "decl" is the illegal redecl, "decls" is the list of previous decls.
    void diagnoseIllegalRedecl(NamedDecl* decl, const NamedDeclVec& decls) {
      // Find the earliest candidate in file
      NamedDecl* earliest = findEarliestInFile(decl->getBegin(), decls);
      // If there's a earliest decl, diagnose. 
      // (We might not have one if this is the first decl)
      if(earliest)
        diagnoseIllegalRedecl(earliest, decl);
    }

    // Helper diagnoseIllegalRedecl
    DiagID getAppropriateDiagForRedecl(NamedDecl* original, NamedDecl* redecl) {
      // This is a "classic" var redeclaration
      if (isVarOrParamDecl(original) && isVarOrParamDecl(redecl)) {
        return isa<ParamDecl>(original) ?
          DiagID::sema_invalid_param_redecl : DiagID::sema_invalid_var_redecl;
      }
      // Invalid function redeclaration
      else if (isa<FuncDecl>(original) && isa<FuncDecl>(redecl))
        return DiagID::sema_invalid_redecl;
      // Redecl as a different kind of symbol
      else
        return DiagID::sema_invalid_redecl_diff_symbol_kind;
    }

    // Diagnoses an illegal redeclaration where "redecl" is an illegal
    // redeclaration of "original"
    void diagnoseIllegalRedecl(NamedDecl* original, NamedDecl* redecl) {
      // Quick checks
      Identifier id = original->getIdentifier();
      assert(original && redecl && "args cannot be null!");
      assert((id == redecl->getIdentifier())
        && "it's a redeclaration but names are different?");

      DiagID diagID = getAppropriateDiagForRedecl(original, redecl);
      getDiags()
        .report(diagID, redecl->getIdentifierRange())
        .addArg(id);
      getDiags()
        .report(DiagID::sema_1stdecl_seen_here, original->getIdentifierRange())
        .addArg(id);
    }

    // Diagnoses an incompatible variable initializer.
    void diagnoseInvalidVarInitExpr(VarDecl* decl, Expr* init) {
      getDiags()
        .report(DiagID::sema_invalid_vardecl_init_expr, init->getRange())
        .addArg(init->getType())
        .addArg(decl->getType())
        .setExtraRange(decl->getTypeRange());
    }

    //----------------------------------------------------------------------//
    // "visit" methods
    //----------------------------------------------------------------------//
    // Theses visit() methods will perform the necessary tasks to check a
    // single declaration.
    //
    // Theses methods may call visit on the children of the declaration, or 
    // call Sema checking functions to perform Typechecking of other node
    // kinds.
    //----------------------------------------------------------------------//

    void visit(Decl* decl) {
      decl->setCheckState(Decl::CheckState::Checking);
      Inherited::visit(decl);
      decl->setCheckState(Decl::CheckState::Checked);
    }

    void visitParamDecl(ParamDecl* decl) {
      // Check this decl for being an illegal redecl
      if (checkForIllegalRedecl(decl)) {
        // Not an illegal redeclaration, add it to the scope
        getSema().addLocalDeclToScope(decl);
      } 
    }

    void visitVarDecl(VarDecl* decl) {
      // Check this decl for being an illegal redecl
      if (checkForIllegalRedecl(decl)) {
        // Not an illegal redeclaration, if it's a local decl,
        // add it to the scope
        if (decl->isLocal())
          getSema().addLocalDeclToScope(decl);
      }

      // Check the init expr
      if (Expr* init = decl->getInitExpr()) {
        // Check the init expr
        bool ok = getSema().typecheckExprOfType(init, decl->getType(), 
          /*allowDowncast*/ false);
        // Replace the expr
        decl->setInitExpr(init);
        // If the type didn't match, diagnose
        if(!ok) {
          // (don't diagnose if we already have an error type to avoid
          // cascading errors)
          if(!init->getType()->is<ErrorType>())
            diagnoseInvalidVarInitExpr(decl, init);
        }
      }
    }

    void visitFuncDecl(FuncDecl* decl) {
      // Tell Sema that we enter this func's scope
      auto scopeGuard = getSema().enterFuncScopeRAII(decl);
      // Check if this is an invalid redecl
      checkForIllegalRedecl(decl);
      // Check it's parameters
      for (ParamDecl* param : *decl->getParams())
        visit(param);
      // And check it's body!
      getSema().checkStmt(decl->getBody());
    }

    void visitUnitDecl(UnitDecl* unit) {
      // Tell Sema that we're inside this unit's DC
      auto dcGuard = getSema().enterDeclCtxtRAII(unit);
      // And just visit every decl inside the UnitDecl
      for(Decl* decl : unit->getDecls())
        visit(decl);
    }

    //----------------------------------------------------------------------//
    // Helper checking methods
    //----------------------------------------------------------------------//
    // Various semantics-related helper methods 
    //----------------------------------------------------------------------//

    // Checks if "decl" is a illegal redeclaration.
    // If "decl" is a illegal redecl, the appropriate diagnostic is emitted
    // and this function returns false.
    // Returns true if "decl" is legal redeclaration or not a redeclaration
    // at all.
    bool checkForIllegalRedecl(NamedDecl* decl) {        
      Identifier id = decl->getIdentifier();
      LookupResult lookupResult;
      // Build Lookup options:
      LookupOptions options;
      // Don't look in the DeclContext if this is a local declaration
      options.canLookInDeclContext = !decl->isLocal();
      options.shouldIgnore = [&](NamedDecl* result){
        // Ignore if result == decl
        if(result == decl) return true;
        // Ignore if result isn't from the same file
        if(result->getFileID() != decl->getFileID()) return true;
        return false;  // else, don't ignore.
      };
      getSema().doUnqualifiedLookup(lookupResult, id, decl->getBegin(),
                                    options);
      // If there are no matches, this cannot be a redecl
      if (lookupResult.size() == 0)
        return true;
      else {
        // if we only have 1 result, and it's a ParamDecl
        NamedDecl* found = lookupResult.getIfSingleResult();
        if (found && isa<ParamDecl>(found) && !isa<ParamDecl>(decl)) {
          assert(decl->isLocal() && "Global declaration is conflicting with "
                 "a parameter declaration?");
          // Redeclaration of a ParamDecl by non ParamDecl is allowed
          // (a variable decl can shadow a parameter)
          return true;
        }
        // Else, diagnose.
        diagnoseIllegalRedecl(decl, lookupResult.getResults());
        decl->setIsIllegalRedecl(true);
        return false;
      }
    }

    //----------------------------------------------------------------------//
    // Other helper methods
    //----------------------------------------------------------------------//
    // Non semantics related helper methods
    //----------------------------------------------------------------------//
    
    // Searches the vector "decls" to return the first decl that was
    // declared before "loc".
    NamedDecl* 
    findEarliestInFile(SourceLoc loc, const NamedDeclVec& decls) {
      assert(decls.size() && "decls.size() > 0");
      NamedDecl* candidate = nullptr;
      FileID file = loc.getFileID();
      for (NamedDecl* decl : decls) {
        assert(decl && "cannot be null!");
        if (decl->getFileID() == file) {
          SourceLoc declLoc = decl->getBegin();
          // If the decl was declared after our loc, ignore it.
          if (loc.getIndex() < declLoc.getIndex())
            continue;
          if (!candidate)
            candidate = decl;
          else {
            SourceLoc candLoc = candidate->getBegin();
            // if decl has been declared before candidate, 
            // decl becomes the candidate
            if (declLoc.getIndex() < candLoc.getIndex())
              candidate = decl;
          }
        }
      }
      return candidate;
    }

    // Return true if decl is a VarDecl or ParamDecl
    bool isVarOrParamDecl(NamedDecl* decl) {
      return isa<ParamDecl>(decl) || isa<VarDecl>(decl);
    }

};

void Sema::checkDecl(Decl* decl) {
  DeclChecker(*this).check(decl);
}