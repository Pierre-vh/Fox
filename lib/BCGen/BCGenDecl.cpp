//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : BCGenDecl.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/BCGen/BCGen.hpp"
#include "Fox/AST/Decl.hpp"
#include "Fox/AST/Stmt.hpp"
#include "Fox/BC/BCBuilder.hpp"
#include "Fox/BC/BCModule.hpp"
#include "Fox/Common/Errors.hpp"

using namespace fox;

//----------------------------------------------------------------------------//
// BCGen Entrypoints
//----------------------------------------------------------------------------//

void BCGen::genFunc(BCModuleBuilder& builder, FuncDecl* func) {
  assert(func && "func is null");
  // For now, only gen the body.
  genStmt(builder, func->getBody());
}

void BCGen::genGlobalVar(BCModuleBuilder&, VarDecl* var) {
  assert(var && "var is null");
  assert((!var->isLocal()) && "var is not global!");
  fox_unimplemented_feature("BCGen::genGlobalVar");
}

std::unique_ptr<BCModule> BCGen::genUnit(UnitDecl* unit) {
  assert(unit && "unit is null");
  BCModuleBuilder theBuilder;
  for (Decl* decl : unit->getDecls()) {
    // BCGen is a WIP, so for now, only gen the first function
    // we find and stop after that.
    if (FuncDecl* fn = dyn_cast<FuncDecl>(decl)) {
      genFunc(theBuilder, fn);
      break;
    }
  }
  return theBuilder.takeModule();
}