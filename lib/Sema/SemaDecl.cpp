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
    
    void visitParamDecl(ParamDecl*) {
      // For ParamDecl, just push them in the local context. There's no
      // need to do anything else.
      // We don't even need to check them for Redecl, that should be done by
      // checkFuncDeclParams
      fox_unimplemented_feature("ParamDecl checking");
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

    void visitFuncDecl(FuncDecl*) {
      auto scopeGuard = getSema().enterLocalScopeRAII();
      // checkFuncDeclParams()
      // check the body of the function
      fox_unimplemented_feature("FuncDecl checking");
    }

    void visitUnitDecl(UnitDecl* decl) {
      auto dcGuard = getSema().enterDeclCtxtRAII(decl);
      // Visit every decl in a any order. No need to check in lexical order.
      fox_unimplemented_feature("UnitDecl checking");
    }

    //----------------------------------------------------------------------//
    // Helper checking methods
    //----------------------------------------------------------------------//
    // Various semantics-related helper methods 
    //----------------------------------------------------------------------//
    
    // TODO: checkFuncDeclParams()
    //    Check that parameters aren't declared twice.
    //    If it isn't an invalid parameter redecl, visit the ParamDecl

    // TODO: bool checkRedecl(VarDecl* decl, LookupResult& results) 
    //    -> if the results == found && the result is a ParamDecl, return true.
    //    -> if the result == ambiguous, diagnose & return false.

    // TODO: NamedDecl* findEarliestInFile(FileID, LookupResult)
};

void Sema::checkDecl(Decl* decl) {
  DeclChecker(*this).check(decl);
}