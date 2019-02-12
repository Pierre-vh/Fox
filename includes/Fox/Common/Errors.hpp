//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : Errors.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains some functions useful in 
// reporting critical errors. Such errors
// include "unreachable code executed" and "bad allocations"
//----------------------------------------------------------------------------//

#pragma once

#include <cassert>

namespace fox {
namespace detail {
  [[noreturn]]
  void fox_unreachable_internal(const char* message,
    const char* file, unsigned line);

  [[noreturn]]
  void fox_unimpl_feat_internal(const char* message,
    const char* file, unsigned line);
} // detail namespace

  // Report a bad alloc error (when an allocator fails)
  // This throws a std::bad_alloc if exceptions are enabled, otherwise,
  // it calls abort().
  void reportBadAlloc(const char* message);
} // fox namespace


// Macro to mark some piece of code as unreachable.
#ifndef fox_unreachable
  #define fox_unreachable(msg) \
    ::fox::detail::fox_unreachable_internal(msg,__FILE__,__LINE__)
#endif

// Macro to mark some functionality as unimplemented/unsupported (for now)
#ifndef fox_unimplemented_feature
  #define fox_unimplemented_feature(msg) \
    ::fox::detail::fox_unimpl_feat_internal(msg,__FILE__,__LINE__)
#endif
