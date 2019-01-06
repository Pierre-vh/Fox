//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
// See the LICENSE.txt file at the root of the project for license information.            
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
  // Currently this will print the error message to cerr
  // and throw std::bad_alloc
  void reportBadAlloc(const char* message);
} // fox namespace


// Macro to mark some piece of code as unreachable.
#ifndef fox_unreachable
  #define fox_unreachable(msg) \
    ::fox::detail::fox_unreachable_internal(msg,__FILE__,__LINE__)
#endif


// Macro to mark some functionality as unimplemented for now.
#ifndef fox_unimplemented_feature
  #define fox_unimplemented_feature(msg) \
    ::fox::detail::fox_unimpl_feat_internal(msg,__FILE__,__LINE__)
#endif
