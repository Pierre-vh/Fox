//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : DebugInfo.hpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#pragma once

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Optional.h"
#include "Fox/Common/LLVM.hpp"
#include "Fox/Common/SourceLoc.hpp"

namespace fox {
  /// This class contains debug information for some instructions in a Buffer of
  /// instructions. This will usually not contain any kind of debug information
  /// for most instructions. It'll only contain debug information for instrs
  /// that can fail at runtime, such as divisions, modulos, builtin function
  /// calls, etc.
  class DebugInfo {
    public:
      /// A pair where the first element is the index of the instruction
      /// and the second is the relevant SourceRange for that instruction.
      using IndexRangePair = std::pair<std::size_t, SourceRange>;

      /// A compare for IndexRangePair that compares only using the first
      /// element of the pair.
      /// FIXME: Long name!
      struct IndexRangePairLessThanComparator {
        bool 
        operator()(const std::size_t lhs, const IndexRangePair& rhs) const {
          return lhs < rhs.first;
        }

        bool 
        operator()(const IndexRangePair& lhs, const std::size_t rhs) const {
          return lhs.first < rhs;
        }

        bool 
        operator()(const IndexRangePair& lhs, const IndexRangePair& rhs) const {
          // Just compare the indexes
          return lhs.first < rhs.first;
        }
      };

      DebugInfo() = default;
      
      /// Make DebugInfo non-copyable
      DebugInfo(const DebugInfo&) = delete;
      DebugInfo& operator=(const DebugInfo&) = delete;

      /// Adds a new SourceRange for an instruction;
      /// \param instrIdx the index of the instruction
      /// \param range the range
      void addSourceRange(std::size_t instrIdx, SourceRange range);
      
      /// \returns if found, the SourceRange of the instruction with index
      /// \p instrIdx.
      Optional<SourceRange> getSourceRange(std::size_t instrIdx);

      /// \returns a read-only view of the sorted vector containing
      /// the SourceRanges of the instructions.
      ArrayRef<IndexRangePair> getRanges() const;
      
    private:
      template<typename T>
      auto ranges_lower_bound(T&& val) {
        return std::lower_bound(
          ranges_.begin(), 
          ranges_.end(), 
          std::forward<T>(val), 
          IndexRangePairLessThanComparator()
        );
      }

      template<typename T>
      auto ranges_upper_bound(T&& val) {
        return std::upper_bound(
          ranges_.begin(), 
          ranges_.end(), 
          std::forward<T>(val), 
          IndexRangePairLessThanComparator()
        );
      }
    
      /// A sorted vector containing SourceRanges for instructions.
      SmallVector<IndexRangePair, 4> ranges_;
      
  };
}