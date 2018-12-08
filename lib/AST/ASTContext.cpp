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

LinearAllocator<>& ASTContext::getSemaAllocator() {
  return semaAllocator_;
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

  allocator_.reset();
  semaAllocator_.reset();
}

void ASTContext::freeCS() {
  semaAllocator_.reset();
}

Identifier ASTContext::getIdentifier(const std::string& str) {
	// Search the entry in the set
	auto it = idents_.insert(str).first;
	assert((it != idents_.end()) && "Insertion error");
	// Create the identifier object and return.
	return Identifier(it->c_str());
}
