//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : ASTContext.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/AST/ASTContext.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"
#include <cstring>

using namespace fox;


ASTContext::ASTContext(SourceManager& srcMgr, DiagnosticEngine& diags):
  sourceMgr(srcMgr), diagEngine(diags) {}

UnitDecl* ASTContext::getMainUnit() {
  return theUnit_;
}

const UnitDecl* ASTContext::getMainUnit() const {
  return theUnit_;
}

void ASTContext::setUnit(UnitDecl* decl) {
  theUnit_ = decl;
}

LinearAllocator<>& ASTContext::getAllocator() {
  return allocator_;
}

LinearAllocator<>& ASTContext::getSemaAllocator() {
  return semaAllocator_;
}

void ASTContext::reset() {
  theUnit_ = nullptr;

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
}

Identifier ASTContext::getIdentifier(const std::string& str) {
	// Search the entry in the set
	auto it = idents_.insert(str).first;
	assert((it != idents_.end()) && "Insertion error");
	// Create the identifier object and return.
	return Identifier(it->c_str());
}

string_view ASTContext::allocateCopy(string_view str) {
  std::size_t size = str.size();
  const char* const buffer = str.data();
  void* const mem = allocator_.allocate(size, alignof(char));
  std::memcpy(mem, buffer, size);
  return string_view(static_cast<char*>(mem), size);
}

bool ASTContext::hadErrors() const {
  return diagEngine.getErrorsCount();
}
