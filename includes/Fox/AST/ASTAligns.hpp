//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : ASTAligns.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains various constants to be used as the minimum
// alignement for some AST classes
//----------------------------------------------------------------------------//

#pragma once

namespace fox {
namespace align {
  constexpr unsigned TypeBaseAlignement = 2;
  // We'll want at least 2 free bits on Expr/Decl and Stmt
  constexpr unsigned ExprAlignement = 4;
  constexpr unsigned DeclAlignement = 4;
  constexpr unsigned StmtAlignement = 4;
}
}