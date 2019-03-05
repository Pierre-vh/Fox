//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : BCGen.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the interface to code generation for Fox, which
// converts a Fox AST into bytecode.
//----------------------------------------------------------------------------//

namespace fox {
  class ASTContext;
  class DiagnosticEngine;
  class BCGen {
    public:
      BCGen(ASTContext& ctxt);

      // TODO: Create the entry point for codegen of basic expressions
      //       maybe "genExpr" is alright. Take a SmallVectorImpl as parameter
      //       to write the expr's data there.
      //       That entry point will be public, but will be made private
      //       once code gen for top-level decls work. (will follow the same
      //       plan as sema)

      DiagnosticEngine& getDiagnosticEngine() const;

      ASTContext& getASTContext() const;

    private:
      ASTContext& ctxt_;
  };
}