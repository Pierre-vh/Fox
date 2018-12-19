//===- MemAlloc.h - Memory allocation functions -----------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
/// \file
///
/// This file defines counterparts of C library allocation functions defined in
/// the namespace 'std'. The new allocation functions crash on allocation
/// failure instead of returning null pointer.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_MEMALLOC_H
#define LLVM_SUPPORT_MEMALLOC_H

#include "llvm/Support/Compiler.h"
#include <cstdlib>

#ifdef LLVM_ENABLE_EXCEPTIONS
  #include <exception>
  // If exceptions are enabled, use <exception> and throw a std::bad_alloc
  #define llvm_bad_alloc() do{throw std::bad_alloc();}while(0);
#else 
  // Don't call the normal error handler. It may allocate memory.
  // Directly write an OOM to stderr and abort.
  #define llvm_bad_alloc() do{\
    char OOMMessage[] = "LLVM ERROR: out of memory (Allocation Failed)\n"; \
    ssize_t written = ::write(2, OOMMessage, strlen(OOMMessage)); \
    (void)written; \
    abort();\
  }while(0);
#endif

namespace llvm {

LLVM_ATTRIBUTE_RETURNS_NONNULL inline void *safe_malloc(size_t Sz) {
  void *Result = std::malloc(Sz);
  if (Result == nullptr)
    llvm_bad_alloc();
  return Result;
}

LLVM_ATTRIBUTE_RETURNS_NONNULL inline void *safe_calloc(size_t Count,
                                                        size_t Sz) {
  void *Result = std::calloc(Count, Sz);
  if (Result == nullptr)
    llvm_bad_alloc();
  return Result;
}

LLVM_ATTRIBUTE_RETURNS_NONNULL inline void *safe_realloc(void *Ptr, size_t Sz) {
  void *Result = std::realloc(Ptr, Sz);
  if (Result == nullptr)
    llvm_bad_alloc();
  return Result;
}

}
#endif