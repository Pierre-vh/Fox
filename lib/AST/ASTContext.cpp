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
void* ASTContext::allocate(std::size_t size, unsigned align, AllocationKind kind) {
  void* mem = getAllocator(kind).allocate(size, align);
  assert(mem && "the allocator returned null memory");
  return mem;
}

void ASTContext::dumpAllocator(AllocationKind alloc) const {
  return const_cast<ASTContext*>(this)->getAllocator(alloc).dump();
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
  permaAllocator_.reset();
}

Identifier ASTContext::getIdentifier(const std::string& str) {
	// Search the entry in the set
	auto it = idents_.insert(str).first;
	assert((it != idents_.end()) && "Insertion error");
	// Create the identifier object and return.
	return Identifier(it->c_str());
}

string_view ASTContext::allocateCopy(string_view str, AllocationKind kind) {
  std::size_t size = str.size();
  const char* const buffer = str.data();
  void* const mem = getAllocator(kind).allocate(size, alignof(char));
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

LinearAllocator& ASTContext::getAllocator(AllocationKind alloc) {
  switch(alloc) {
    case AllocationKind::Perma: return permaAllocator_;
    case AllocationKind::Temp:
      assert(!tempAllocators_.empty() && "no temporary allocator available");
      return tempAllocators_.top();
    default: fox_unreachable("unknown allocator kind");
  }
}

RAIITemporaryAllocator::RAIITemporaryAllocator(ASTContext& ctxt) : ctxt_(ctxt) {
  ctxt_.tempAllocators_.emplace();
}

RAIITemporaryAllocator::~RAIITemporaryAllocator() {
  ctxt_.tempAllocators_.pop();
}
