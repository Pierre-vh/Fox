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

#include <iostream>

using namespace fox;


class Sema::DeclChecker : Checker, DeclVisitor<DeclChecker, void> {
  using Inherited = DeclVisitor<DeclChecker, void>;
  friend class Inherited;
  public:
    DeclChecker(Sema& sema) : Checker(sema) {}

    void check(Decl* decl) {
      assert(decl && "cannot have a nullptr argument");
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
    void diagnoseIllegalRedecl(NamedDecl* decl, std::vector<NamedDecl*> decls) {
      // Find the earliest candidate in file
      NamedDecl* earliest = findEarliestInFile(decl->getFile(), decls);
      assert(earliest && "Couldn't find earliest declaration");
      diagnoseIllegalRedecl(earliest, decl);
    }

    // Diagnoses an illegal redeclaration where "redecl" is an illegal
    // redeclaration of "original"
    void diagnoseIllegalRedecl(NamedDecl* original, NamedDecl* redecl) {
      assert(original && redecl && "args cannot be null!");
      Identifier id = original->getIdentifier();
      assert((id == redecl->getIdentifier())
        && "it's a redeclaration but names are different?");
      DiagID diagID = isa<ParamDecl>(original) ? 
        DiagID::sema_invalid_param_redecl : DiagID::sema_invalid_redecl;
      getDiags().report(diagID, redecl->getRange())
        .addArg(id);
      getDiags().report(DiagID::sema_1stdecl_seen_here, original->getRange())
        .addArg(id);
    }

    // Diagnoses an incompatible variable initializer.
    void diagnoseInvalidVarInitExpr(VarDecl* decl, Expr* init) {
      getDiags()
        .report(DiagID::sema_invalid_vardecl_init_expr, init->getRange())
        .addArg(init->getType())
        .addArg(decl->getType())
        .setExtraRange(decl->getTypeLoc().getRange());
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
    
    void visitParamDecl(ParamDecl* decl) {
      // For ParamDecl, just push them in the local context. There's no
      // need to do anything else.
      // We don't even need to check them for Redecl, that is done by
      // checkFuncDeclParams before calling visitParamDecl
      getSema().addLocalDeclToScope(decl);
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
        auto res = getSema().typecheckExprOfType(init, decl->getType(), false);
        // Replace the expr
        decl->setInitExpr(res.second);
        auto flag = res.first;
        using CER = Sema::CheckedExprResult;
        switch (flag) {
          case CER::Ok:
          case CER::Error:
            // Don't diagnose if the expression is correct, of if the expression
            // is an ErrorType (to avoid error cascades)
            break;
          case CER::Downcast:
          case CER::NOk:
            diagnoseInvalidVarInitExpr(decl, init);
            break;
        }
      }
    }

    void visitFuncDecl(FuncDecl* decl) {
      // Tell Sema that we enter this func's scope
      auto scopeGuard = getSema().enterLocalScopeRAII();
      // Check it's parameters
      checkFuncDeclParams(decl);
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
    
    // Marks a ValueDecl as being an invalid redeclaration
    void markAsIllegalRedecl(ValueDecl* decl) {
      decl->setIsIllegalRedecl(true);
    }

    // Calls visit() on the parameters of a FuncDecl, checking for duplicate
    // parameters and emitting diagnostics if required.
    //
    // Returns false if diagnosics were emitted.
    void checkFuncDeclParams(FuncDecl* decl) {
      std::map<Identifier, ParamDecl*> seenParams;
      for(auto param : decl->getParams()) {
        Identifier id = param->getIdentifier();
        auto it = seenParams.find(id);
        // If the identifier isn't a duplicate
        if(it == seenParams.end()) {
          // Add it to the map & visit the decl
          seenParams[id] = param;
          visitParamDecl(param);
        }
        // if the identifier is a duplicate
        else {
          diagnoseIllegalRedecl(it->second, param);
          markAsIllegalRedecl(param);
        }
      }
    }

    // Checks if "decl" is a illegal redeclaration.
    // If "decl" is a illegal redecl, the appropriate diagnostic is emitted
    // and this function returns false.
    // Returns true if "decl" is legal redeclaration or not a redeclaration
    // at all.
    bool checkForIllegalRedecl(VarDecl* decl) {
      using LRK = LookupResult::Kind;
        
      Identifier id = decl->getIdentifier();
      LookupResult lookupResult;
      getSema().doUnqualifiedLookup(lookupResult, id);

      switch (lookupResult.getKind()) {
        case LRK::NotFound:
          // Can't be a redecl
          return true;
        case LRK::Found:
          {
            NamedDecl* found = lookupResult.getIfSingleResult();
            assert(found && "lookupResult's kind is Found but getIfSingleResult "
                   "returns nullptr?");
            if (isa<ParamDecl>(found)) {
              assert(decl->isLocal() && "Global declaration is conflicting with "
                     "a parameter declaration?");
              // Redeclaration of a ParamDecl is allowed
              return true;
            }
          }
          // fall through
        case LRK::Ambiguous:
          // It's an invalid redeclaration: diagnose, mark it and
          // return false;
          diagnoseIllegalRedecl(decl, lookupResult.getResults());
          decl->setIsIllegalRedecl(true);
          return false;
        default:
          fox_unreachable("all cases handled");
      }
    }

    //----------------------------------------------------------------------//
    // Other helper methods
    //----------------------------------------------------------------------//
    // Non semantics related helper methods
    //----------------------------------------------------------------------//
    
    // Searches the vector of Decl to find the earliest declaration
    // in the FileID "file"
    NamedDecl* 
    findEarliestInFile(FileID file, const std::vector<NamedDecl*>& decls) {
      assert(decls.size() && "decls.size() > 0");
      NamedDecl* candidate = nullptr;
      for (NamedDecl* decl : decls) {
        assert(decl && "cannot be null!");
        if (decl->getFile() == file) {
          if (!candidate)
            candidate = decl;
          else {
            SourceLoc candLoc = candidate->getRange().getBegin();
            SourceLoc declLoc = decl->getRange().getBegin();
            // if decl has been declared before candidate, 
            // decl becomes the candidate
            if (SourceLoc::CompareByIndex()(declLoc, candLoc))
              candidate = decl;
          }
        }
      }
      return candidate;
    }
};

void Sema::checkDecl(Decl* decl) {
  DeclChecker(*this).check(decl);
}