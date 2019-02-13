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
  return const_cast<ASTContext*>(this)->getAllocator().dump();
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

Identifier ASTContext::getIdentifier(const std::string& str) {
	// Search (or create) the entry in the set
	auto it = idents_.insert(str).first;
	assert((it != idents_.end()) && "Insertion error");
  // return it
	return (it->c_str());
}

string_view ASTContext::allocateCopy(string_view str) {
  std::size_t size = str.size();
  const char* const buffer = str.data();
  void* const mem = permaAllocator_.allocate(size, alignof(char));
  std::memcpy(mem, buffer, size);
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

LinearAllocator& ASTContext::getAllocator() {
  return permaAllocator_;
}
