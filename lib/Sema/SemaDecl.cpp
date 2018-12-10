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
      auto declCtxtGuard = enterDeclCtxt(decl);
      // Sema::setDeclCtxtRAII(decl)
      // visit(decl parameters)
      // Sema::checkNode(decl->getBody())
      fox_unimplemented_feature("FuncDecl checking");
    }

    void visitUnitDecl(UnitDecl* decl) {
      auto declCtxtGuard = enterDeclCtxt(decl);
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

    Sema::RAIIDeclCtxt enterDeclCtxt(DeclContext* dc) {
      return getSema().setDeclCtxtRAII(dc);
    }

    // CheckValueDecl

    // TODO: Method to check if a ValueDecl hasn't been declared already
      // Be careful: We will always have at least 1 result because the
      //             Decl will have been registered by the Parser already.
      //             Just check that the only result found matches this 
      //             ValueDecl* ptr. 
      //             (Convert both to Decl and pointer compare)
      //             if results == 0 -> unreachable
      //             if results > 1 -> diagnose 
      // TODO: Define if we diagnose every conflicting Decl in
      // one go (then we must mark them so we won't diagnose them again later),
      // or if we do it one at a time. If we do it one at a time, we must
      // have a way of knowing which one was the very first declaration 
      // 

    // TODO: Method to register a decl in the current scope.
};

void Sema::checkDecl(Decl* decl) {
  DeclChecker(*this).check(decl);
}