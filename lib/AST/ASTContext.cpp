//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
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

ASTContext::~ASTContext() {
  reset();
}

UnitDecl* ASTContext::getMainUnit() {
  return theUnit_;
}

const UnitDecl* ASTContext::getMainUnit() const {
  return theUnit_;
}

void ASTContext::setUnit(UnitDecl* decl) {
  theUnit_ = decl;
}

LLVM_ATTRIBUTE_RETURNS_NONNULL LLVM_ATTRIBUTE_RETURNS_NOALIAS
void* ASTContext::allocate(std::size_t size, unsigned align) {
  void* mem = allocator_.allocate(size, align);
  assert(mem && "the allocator returned null memory");
  return mem;
}

const LinearAllocator& ASTContext::getAllocator() const {
  return allocator_;
}

void ASTContext::reset() {
  // Clear sets/maps
  arrayTypes_.clear();
  lvalueTypes_.clear();
  functionTypes_.clear();
  idents_.clear();

  // Clear type singletons
  theUnit_ = nullptr;
  theIntType_ = nullptr;
  theFloatType_ = nullptr;
  theCharType_ = nullptr;
  theBoolType_ = nullptr;
  theStringType_ = nullptr;
  theVoidType_ = nullptr;
  theErrorType_ = nullptr;

  // Call the cleanups methods
  callCleanups();

  // Reset the allocator, freeing it's memory.
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

void ASTContext::addCleanup(std::function<void(void)> fn) {
  cleanups_.push_back(fn);
}

void ASTContext::callCleanups() {
  for(auto cleanup : cleanups_) 
    cleanup();
  cleanups_.clear();
}
