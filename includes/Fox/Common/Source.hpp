//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : Source.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the SourceManager, SourceLoc and SourceRange
// classes.
//----------------------------------------------------------------------------//

#pragma once

#include "string_view.hpp"
#include "LLVM.hpp"
#include "llvm/ADT/SmallVector.h"
#include <cstdint>
#include <memory>
#include <string>
#include <map>
#include <tuple>

namespace fox {
  class SourceLoc;
  class SourceManager;

  // The FileID is an opaque object that packs a 16 bits int which
  // represents a file index inside the SourceManager.
  class FileID {
    public:
      using IDTy = std::uint16_t;
      static constexpr IDTy npos = std::numeric_limits<IDTy>::max();

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

      // Returns the raw index contained inside the FileID
      // which represents the file inside the SourceManager.
      //
      // If the index is equal to npos, the FileID is
      // considered invalid.
      IDTy getRaw() const;

    protected:
      friend class SourceManager;

      // Only the SourceManager can create valid FileIDs
      FileID(std::size_t value);

    private:
      IDTy value_ = npos;
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
      using IndexTy = std::uint32_t;

      // Creates a new, invalid SourceLoc.
      SourceLoc();
      explicit SourceLoc(FileID fid, IndexTy idx = 0);

      bool isValid() const;
      explicit operator bool() const; // ShortCut for isValid

      bool operator ==(const SourceLoc other) const;
      bool operator !=(const SourceLoc other) const;

      // Returns the ID of the file in which this SourceLoc lives.
      FileID getFileID() const;

      // Returns the raw index of the SourceLoc.
      //
      // Note that this should never be used in place of a SourceLoc since
      // it doesn't preserve the FileID.
      IndexTy getRawIndex() const;

      // Returns true if this SourceLoc comes before the other SourceLoc.
      // Returns false if the other SourceLoc doesn't belong to the 
      // same file, or if it comes after this one.
      bool comesBefore(SourceLoc other) const;

      // Returns a string representation of a SourceLoc:
      //  Format: line:column
      //  Example: 3:4
      //
      // Returns "" if this SourceLoc is invalid.
      std::string toString(const SourceManager& srcMgr) const;

    private:
      FileID fid_;
      IndexTy idx_;
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

      // Returns a string representation of a SourceLoc:
      //  Format: 
      //    when offset == 0: line:column
      //    when begLine == endLine: line:column-column
      //    when begLine != endLine: line:column-line:column
      //  Example: 3:4, 3:4-6, 3:4-4:5
      //
      // Returns "" if this SourceRange is invalid.
      std::string toString(const SourceManager& srcMgr) const;

    private:
      SourceLoc sloc_;
      OffsetTy offset_;
  };

  // POD-like struct containing a human-readable source loc information.
  struct CompleteLoc {
    using LineTy = std::uint32_t;
    using ColTy = std::uint16_t;

    CompleteLoc(const std::string& fName, LineTy ln, ColTy col)
      : fileName(fName), line(ln), column(col) {}

    bool operator==(const CompleteLoc& other) const {
      return (fileName == other.fileName) 
        && (line == other.line) && (column == other.column);
    }

    std::string fileName;
    LineTy line;
    ColTy column;
  };

  // The SourceManager, which manages source files. It reads
  // them, stores them and gives them a unique FileID.
  class SourceManager {
    public:
      SourceManager() = default;

      // Return enum for readFile
      enum class FileStatus : std::uint8_t {
        // The file was successfully read and loaded in memory.
        // It's FileID is the first element of the pair.
        Ok,
        // The file couldn't be found.
        NotFound,
        // The file had an invalid encoding. Currently, only
        // ASCII and UTF8 are supported.
        InvalidEncoding
      };

      // Load a file in memory. 
      //  Returns a pair. The first element is the FileID, it'll evaluate
      //  to false if the file was not loaded in memory.
      //  The second element contains a more detailed status (see FileStatus)
      std::pair<FileID, FileStatus> readFile(string_view path);

      // Load a string in the SM. First arg is the string to load, 
      // the second is the name we should give to the file.
      FileID loadFromString(string_view str,
                            string_view name = "<unknown>");

      // Returns a string_view of the Source file's content.
      // The FileID MUST be valid.
      string_view getNameOfFile(FileID fid) const;

      // Returns a string_view of the Source file's name.
      // The FileID MUST be valid.
      string_view getContentsOfFile(FileID fid) const;

      // Returns the line number of a SourceLoc
      CompleteLoc::LineTy getLineNumber(SourceLoc loc) const;

      // Returns a SourceRange that covers a whole file. 
      // It begins at index 0, and ends at the last character.
      SourceRange getRangeOfFile(FileID file) const;

      // Requests the human-readable location a SourceLoc points to.
      // This function will assert that the SourceLoc is valid;
      // This function accepts a SourceLoc that points right past 
      // the end of the file.
      // Any value greater than that will trigger an assertion ("out of range")
      CompleteLoc getCompleteLoc(SourceLoc sloc) const;

      // Returns the complete line of source code for a given SourceLoc
      // An optional argument (pointer) can be passed. If it is present,
      // the function will store the Index at which the line begins in 
      // this variable.
      string_view getSourceLine(SourceLoc loc, 
        SourceLoc::IndexTy* lineBeg = nullptr) const;

      // Returns a SourceLoc that refers to the next code point
      // after "loc".
      SourceLoc getNextCodepointSourceLoc(SourceLoc loc) const;

    private:
      // This class represents the data that is stored internally inside the
      // SourceManager.
      // 
      // TODO: Use Pimpl to move that out of the header and greatly reduce
      // the includes (remove SmallVector, unique_ptr & map from the includes)
      struct Data {
        Data(string_view name, string_view content)
          : name(name.to_string()), content(content.to_string()) {}

        template<typename Iterator>
        Data(string_view name, Iterator begin, Iterator end)
          : name(name.to_string()), content(begin, end) {}

        const std::string name;
        const std::string content;
        protected:
          using IndexTy = SourceLoc::IndexTy;
          using LineTy = CompleteLoc::LineTy;

          friend class SourceManager;

          // This is the cached "line table", which is used to efficiently
          // calculate the line number of a SourceLoc.
          mutable std::map<IndexTy, LineTy> lineTable_;

          // Flag indicating whether we have calculated the LineTable.
          mutable bool calculatedLineTable_ = false;

          // We cache the last line table search here.
          // The first element of the pair is the sourceloc that we
          // searched for, the second is the result we returned.
          mutable std::pair<SourceLoc, std::pair<IndexTy, LineTy>> 
            lastLTSearch_;
      };

      // Returns a pointer to the "Data" for a given File.
      // The result is always non null (guaranteed by an assertion)
      // The result will also always be constant because the data stored
      // is immutable.
      const Data* getData(FileID fid) const;

      // Calculates the "line table" of a given Data.
      void calculateLineTable(const Data* data) const;

      // Checks if a SourceLoc's index refers to a valid position
      // (or a past-the-end position) in the data.
      bool isIndexValid(const Data* data, SourceLoc::IndexTy idx) const;

      // Searches the "line table" of a given Data.
      std::pair<SourceLoc::IndexTy, CompleteLoc::LineTy>
      searchLineTable(const Data* data, const SourceLoc& loc) const;

      // Inserts a new Data in the datas_ vector, returning it's FileID.
      FileID insertData(std::unique_ptr<Data> data);

      // Make it non copyable
      SourceManager(const SourceManager&) = delete;
      SourceManager& operator=(const SourceManager&) = delete;
      
      // Member variables
      SmallVector<std::unique_ptr<Data>, 4> datas_;
  };
  
  // Converts a SourceManager::FileStatus to a string.
  std::string toString(SourceManager::FileStatus status);
}
