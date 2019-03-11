//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : BCGen.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/BCGen/BCGen.hpp"
#include "Fox/AST/ASTContext.hpp"

using namespace fox;

BCGen::BCGen(ASTContext& ctxt) : ctxt(ctxt), diagEngine(ctxt.diagEngine) {}