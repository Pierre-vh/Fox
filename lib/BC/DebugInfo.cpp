//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : BCModule.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/BC/DebugInfo.hpp"
#include "llvm/ADT/ArrayRef.h"
#include <algorithm>

using namespace fox;

void DebugInfo::addSourceRange(std::size_t instrIdx, SourceRange range) {
  // Check that we don't already have something for this instr
  assert(!getSourceRange(instrIdx).hasValue()
    && "Already has debug info for this instruction!");
  // Find the upper bound & insert the element before it.
  auto ub = ranges_upper_bound(instrIdx);
  ranges_.insert(ub, {instrIdx, range});
}

Optional<SourceRange> DebugInfo::getSourceRange(std::size_t instrIdx) {
  if(ranges_.size() == 0) 
    return None;

  auto ptr = ranges_lower_bound(instrIdx);
  // Only return "direct" hits.
  if(ptr->first == instrIdx) 
    return ptr->second;
  return None;
}

ArrayRef<DebugInfo::IndexRangePair> DebugInfo::getRanges() const {
  return ranges_;
}
