//===- MemAlloc.h - Memory allocation functions -----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
///
/// This file defines counterparts of C library allocation functions defined in
/// the namespace 'std'. The new allocation functions crash on allocation
/// failure instead of returning null pointer.
///
//===----------------------------------------------------------------------===//
//
// Modifications made to this file for the Fox Project:
//  1 - Removed ErrorHandling.h include
//  2 - Added lines 29-42: Define llvm_bad_alloc directly in this file to
//      remove the need for ErrorHandling.h/.cpp
//  3 - Replaced calls to report_bad_alloc at lines 51, 59, 66 with calls to
//      llvm_bad_alloc
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_MEMALLOC_H
#define LLVM_SUPPORT_MEMALLOC_H

#include "llvm/Support/Compiler.h"
#include <cstdlib>

#ifdef LLVM_ENABLE_EXCEPTIONS
  #include <exception>
  // If exceptions are enabled, use <exception> and throw a std::bad_alloc
  #define llvm_bad_alloc(msg) do{throw std::bad_alloc();}while(0);
#else 
  // Directly write an OOM to stderr and abort.
  #define llvm_bad_alloc(msg) do{\
    char OOMMessage[] = "LLVM ERROR: out of memory (Allocation Failed):" 
                        + msg + "\n"; \
    ssize_t written = ::write(2, OOMMessage, strlen(OOMMessage)); \
    (void)written; \
    abort();\
  }while(0);
#endif

namespace llvm {

LLVM_ATTRIBUTE_RETURNS_NONNULL inline void *safe_malloc(size_t Sz) {
  void *Result = std::malloc(Sz);
  if (Result == nullptr)
    llvm_bad_alloc("Allocation failed");
  return Result;
}

LLVM_ATTRIBUTE_RETURNS_NONNULL inline void *safe_calloc(size_t Count,
                                                        size_t Sz) {
  void *Result = std::calloc(Count, Sz);
  if (Result == nullptr)
    llvm_bad_alloc("Allocation failed");
  return Result;
}

LLVM_ATTRIBUTE_RETURNS_NONNULL inline void *safe_realloc(void *Ptr, size_t Sz) {
  void *Result = std::realloc(Ptr, Sz);
  if (Result == nullptr)
    llvm_bad_alloc("Allocation failed");
  return Result;
}

}
#endif