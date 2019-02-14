//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : SourceLoc.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the SourceLoc and SourceRange classes.
//----------------------------------------------------------------------------//

#pragma once

#include <cstddef>
#include <cstdint>

namespace fox {
  class SourceManager;

  // The FileID is an opaque object that packs a 16 bits int which
  // represents a file index inside the SourceManager.
  class FileID {
    public:
      using IDTy = std::uint16_t;
      static constexpr IDTy npos = 0xFFFF;

      // Creates a new invalid FileID.
      FileID() = default;

      // Checks if this FileID should be considered valid.
      bool isValid() const;

      // Calls isValid()
      explicit operator bool() const;

      bool operator ==(const FileID other) const;
      bool operator !=(const FileID other) const;

      // For use in containers
      bool operator <(const FileID other) const;

    private:
      friend class SourceManager;

      // Only the SourceManager can create valid FileIDs
      FileID(std::size_t value);

      IDTy idx_ = npos;
  };

  // The SourceLoc is a lightweight wrapper around a FileID and an index
  // which, combined, locate a byte in the source file. 
  //
  // The byte can be a single ASCII character, or the beginning of a UTF-8
  // code point.
  class SourceLoc {
    public:
      // Use 32 bits int for the Index. This shoud allow the interpreter
      // to load and work with files up to 4GB alone. This can be upgraded
      // to a int64 if needed.
      using index_type = std::uint32_t;

      // Creates a new, invalid SourceLoc.
      SourceLoc();
      explicit SourceLoc(FileID fid, index_type idx = 0);

      bool isValid() const;
      explicit operator bool() const; 

      bool operator ==(const SourceLoc other) const;
      bool operator !=(const SourceLoc other) const;

      // Returns the ID of the file in which this SourceLoc lives.
      FileID getFileID() const;

      // Returns the raw index of the SourceLoc.
      //
      // Note that this should never be used in place of a SourceLoc since
      // it doesn't preserve the FileID.
      index_type getRawIndex() const;

      // Returns true if this SourceLoc comes before the other SourceLoc.
      // Returns false if the other SourceLoc doesn't belong to the 
      // same file, or if it comes after this one.
      bool comesBefore(SourceLoc other) const;

    private:
      FileID fid_;
      index_type idx_;
  };

  // The SourceRange is a wrapper around a SourceLoc and an offset, 
  // which combined represent a range of bytes in the file.
  //
  // IMPORTANT: The SourceRange IS NOT a half-open range. This means that
  // the first loc represents the first byte in the range, and the last loc
  // represents the last byte in the range.
  //
  //  e.g. If the file content is "foo bar":
  //    auto loc = SourceLoc(fileID, 0) refers to "f"
  //    SourceRange(loc, 0) refers to "f" (.getBegin() == .getEnd())
  //    SourceRange(loc, 2) refers to "foo"
  class SourceRange {
    public:
      using OffsetTy = std::uint32_t;

      explicit SourceRange(SourceLoc sloc, OffsetTy offset = 0);
      explicit SourceRange(SourceLoc a, SourceLoc b);
      SourceRange();

      bool isValid() const;
      explicit operator bool() const; // Shortcut for isValid

      SourceLoc getBegin() const;
      SourceLoc getEnd() const;
      FileID getFileID() const;

      // Returns the raw offset contained inside this SourceRange.
      OffsetTy getRawOffset() const;

      // Returns true if this SourceRange only covers one characters
      // (and thus can be converted to a SourceLoc without losing
      // information)
      bool isOnlyOneCharacter() const;

      // Return true if the SourceLoc is contained inside this SourceRange.
      // (beg >= loc <= end)
      // Return false if the SourceLoc doesn't belong to the same file,
      // or if it's not contained inside this SourceRange
      bool contains(SourceLoc loc) const;

      // Returns true if the SourceRange is contained inside this SourceRange.
      // (contains(range.getBegin()) && (contains(range.getEnd()))
      // Return false if the SourceRange doesn't belong to the same file,
      // or if it's not contained inside this SourceRange
      bool contains(SourceRange range) const;

    private:
      SourceLoc sloc_;
      OffsetTy offset_;
  };
}