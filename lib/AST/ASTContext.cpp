//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : ASTContext.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/AST/ASTContext.hpp"

using namespace fox;

ASTContext::ASTContext() {

}

UnitDecl* ASTContext::getMainUnit() {
  return mainUnit_;
}

void ASTContext::addUnit(UnitDecl* unit, bool isMainUnit) {
  units_.push_back(unit);

  if (isMainUnit)
    mainUnit_ = unit;
}

LinearAllocator<>& ASTContext::getAllocator() {
  return allocator_;
}

LinearAllocator<>& ASTContext::getCSAllocator() {
  return csAllocator_;
}

void ASTContext::reset() {
  units_.clear();
  mainUnit_ = nullptr;

  // Clear maps of ArrayType/LValueTypes
  arrayTypes.clear();
  lvalueTypes.clear();

  theIntType = nullptr;
  theFloatType = nullptr;
  theCharType = nullptr;
  theBoolType = nullptr;
  theStringType = nullptr;
  theVoidType = nullptr;
  theErrorType = nullptr;

  theArrayCS = nullptr;

  allocator_.reset();
  csAllocator_.reset();
}

void ASTContext::freeCS() {
  csAllocator_.reset();
}