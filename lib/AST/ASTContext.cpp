//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
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

LLVM_ATTRIBUTE_RETURNS_NONNULL LLVM_ATTRIBUTE_RETURNS_NOALIAS
void* ASTContext::allocate(std::size_t size, unsigned align) {
  void* mem = permaAllocator_.allocate(size, align);
  assert(mem && "the allocator returned null memory");
  return mem;
}

void ASTContext::dumpAllocator() const {
  return permaAllocator_.dump();
}

void ASTContext::reset() {
  // Clear sets/maps
  arrayTypes_.clear();
  lvalueTypes_.clear();
  functionTypes_.clear();
  idents_.clear();

  // Clear type singletons
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
  permaAllocator_.reset();
}

Identifier ASTContext::getIdentifier(string_view str) {
	// Search the identifiers set (this will use the hash
  // of the string, not its pointer)
	auto it = idents_.find(str);
  // Found, return.
  if(it != idents_.end())
    return Identifier(it->data());
  // The entry doesn't exist yet, so allocate a copy of 
  // the string in the ASTContext and insert it in the set.
  str = allocateCopy(str);
  idents_.insert(str);
  // Return an identifier object using a pointer to that
  // allocated string.
  return Identifier(str.data());
}

string_view ASTContext::allocateCopy(string_view str) {
  std::size_t size = str.size();
  assert(size > 0 && "string is empty");
  const char* buffer = str.data();
  // Allocate a block of the size of the string + 1, so we can add
  // a null terminator.
  char* mem = static_cast<char*>(permaAllocator_.allocate(size+1));
  std::memcpy(mem, buffer, size);
  // Add a null terminator
  static_cast<char*>(mem)[str.size()] = 0;
  return string_view(static_cast<char*>(mem), size);
}

void ASTContext::addCleanup(std::function<void(void)> fn) {
  cleanups_.push_back(fn);
}

void ASTContext::callCleanups() {
  for(const auto& cleanup : cleanups_)
    cleanup();
  cleanups_.clear();
}