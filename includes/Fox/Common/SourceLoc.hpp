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
#include <iosfwd>

namespace fox {
  class SourceManager;

  /// The FileID is an opaque object that represents an unique identifier given
  /// to a file stored in a SourceManageR.
  class FileID {
    using IDTy = std::uint16_t;
    static constexpr IDTy npos = 0xFFFF;
    public:
      /// Creates a new, invalid FileID.
      FileID() = default;

      /// \returns true if this FileID is valid.
      bool isValid() const;

      /// \returns isValid()
      explicit operator bool() const;

      bool operator ==(const FileID other) const;
      bool operator !=(const FileID other) const;
      bool operator <(const FileID other) const;

      friend std::ostream& operator<<(std::ostream& os, FileID file);

    private:
      friend class SourceManager;

      // Only the SourceManager can create valid FileIDs
      FileID(std::size_t value);

      IDTy idx_ = npos;
  };

  /// The SourceLoc packs a FileID with an offset in the source file.
  /// Together, they represent a byte in the source file. That byte can
  /// be an ASCII character or the beginning of a UTF-8 codepoint.
  class SourceLoc {
    public:
      /// The type of the offset/index
      using IndexTy = std::uint32_t;

      /// Creates a new, invalid SourceLoc.
      SourceLoc();
      /// Creates a SourceLoc from a FileID and an index.
      explicit SourceLoc(FileID fid, IndexTy idx = 0);

      /// \returns true if this SourceLoc is valid
      bool isValid() const;
      /// \returns isValid()
      explicit operator bool() const; 

      bool operator ==(const SourceLoc other) const;
      bool operator !=(const SourceLoc other) const;

      /// \returns the FileID of this SourceLoc
      FileID getFileID() const;
      /// \returns the raw offset/index
      IndexTy getRawIndex() const;

      /// \returns true if this SourceLoc comes before the other SourceLoc.
      bool comesBefore(SourceLoc other) const;

      friend std::ostream& operator<<(std::ostream& os, SourceLoc loc);

    private:
      friend class SourceRange;
      FileID fid_;
      IndexTy idx_;
  };

  /// The SourceRange is a wrapper around a SourceLoc and another offset, 
  /// which combined represent a range of bytes in the file.
  ///
  /// The SourceRange IS NOT a half-open range. This means that
  /// the first loc represents the first byte in the range, and the last loc
  /// represents the last byte in the range. Note that that last byte may
  /// be the first byte of an UTF-8 codepoint too.
  /// \verbatim
  ///  e.g. If the file content is "foo bar":
  ///    auto loc = SourceLoc(fileID, 0) refers to "f"
  ///    SourceRange(loc, 0) refers to "f" (.getBeginLoc() == .getEndLoc())
  ///    SourceRange(loc, 2) refers to "foo"
  /// \endverbatim
  class SourceRange {
    public:
      /// The type of the offset
      using OffsetTy = SourceLoc::IndexTy;

      /// Creates a SourceRange from a SourceLoc \p sloc and a
      /// "raw" offset \p offset
      explicit SourceRange(SourceLoc sloc, OffsetTy offset = 0);
      /// Creates a SourceRange from 2 SourceLocs \p a and \p b
      explicit SourceRange(SourceLoc a, SourceLoc b);
      /// Creates an invalid SourceRange
      SourceRange();

      /// \returns true if this SourceRange is valid
      bool isValid() const;
      /// \returns isValid()
      explicit operator bool() const;

      SourceLoc getBeginLoc() const;
      SourceLoc getEndLoc() const;
      FileID getFileID() const;

      /// \returns the raw offset 
      OffsetTy getRawOffset() const;

      /// \returns true if the SourceLoc is contained inside this SourceRange.
      /// (getBeginLoc() >= \p loc <= getEndLoc())
      bool contains(SourceLoc loc) const;

      /// \returns true if the SourceRange is contained inside this SourceRange.
      /// (contains(range.getBeginLoc()) && (contains(range.getEndLoc()))
      bool contains(SourceRange range) const;

      friend std::ostream& operator<<(std::ostream& os, SourceRange range);

    private:
      SourceLoc loc_;
      OffsetTy offset_;
  };
}