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

    // TODO: diagnoseInvalidRedecl(VarDecl* decl, LookupResult&)
    //   multiple results:
    //      ->  findEarliestInFile()
    //   then, depending on the kind of the result
    //      -> if VarDecl, diagnose "invalid redecl of" ... + note on first decl
    //      -> if FuncDecl, diagnose that this variable shares the same name
    //         as a func + note on the FuncDecl

    void diagnoseInvalidParamRedecl(ParamDecl* original, ParamDecl* redecl) {
      Identifier id = original->getIdentifier();
      assert((id == redecl->getIdentifier())
        && "it's a redeclaration but names are different?");
      auto& diags = getDiags();
      diags.report(DiagID::sema_invalid_param_redecl, redecl->getRange())
        .addArg(id);
      diags.report(DiagID::sema_1stdecl_seen_here, original->getRange())
        .addArg(id);
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
      getSema().addToScope(decl);
    }

    void visitVarDecl(VarDecl*) {
      // Check the init if there's one.
      //    TODO LATER: Disable function calls and declrefs inside function
      //        initializer of global variables.
      //
      // Check VarDecl for invalid Redeclaration by performing a lookup
      //
      // If the LookupResult's kind != "NotFound", call checkRedecl() and
      //  stop checking if it returns false
      // 
      // Then, if everything's alright, register it in the scope
      fox_unimplemented_feature("VarDecl checking");
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
          diagnoseInvalidParamRedecl(it->second, param);
          markAsIllegalRedecl(param);
        }
      }
    }

    // TODO: bool checkRedecl(VarDecl* decl, LookupResult& results) 
    //    -> if the results == found && the result is a ParamDecl, return true.
    //    -> if the result == ambiguous, diagnose & return false.

    // TODO: NamedDecl* findEarliestInFile(FileID, LookupResult)
};

void Sema::checkDecl(Decl* decl) {
  DeclChecker(*this).check(decl);
}