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
      // do checkValueDecl();
      fox_unimplemented_feature("ParamDecl checking");
    }

    void visitVarDecl(VarDecl*) {
      // do checkValueDecl();
      fox_unimplemented_feature("VarDecl checking");
    }

    void visitFuncDecl(FuncDecl* decl) {
      // FuncDecl is a DeclCtxt. Since we're checking it, set the currently 
      // active DeclContext in Sema to us.
      auto dcGuard = getSema().setDeclCtxtRAII(decl);
      // FuncDecl is also a local scope, so create a new scope.
      assert(decl->isLocalDeclContext() && "FuncDecl isn't local?");
      auto scopeGuard = getSema().enterNewLocalScopeRAII();
      // visit(decl parameters)
      // Sema::checkNode(decl->getBody())
      fox_unimplemented_feature("FuncDecl checking");
    }

    void visitUnitDecl(UnitDecl* decl) {
      // UnitDecl is a DeclCtxt. Since we're checking it, set the currently 
      // active DeclContext in Sema to us.
      auto dcGuard = getSema().setDeclCtxtRAII(decl);
      // Sema::setDeclCtxtRAII(decl)
      // visit(decl parameters)
      // Sema::checkNode(decl->getBody())
      fox_unimplemented_feature("UnitDecl checking");
    }

    //----------------------------------------------------------------------//
    // Helper checking methods
    //----------------------------------------------------------------------//
    // Various semantics-related helper methods 
    //----------------------------------------------------------------------//
    

    // Checks a ValueDecl for Redeclaration
    bool checkValueDeclForRedecl(ValueDecl* decl) {
      // Lookup, check that Zero result.
      // In the future, check here for overload-related
      // stuff.
      return false;
    }
};

void Sema::checkDecl(Decl* decl) {
  DeclChecker(*this).check(decl);
}