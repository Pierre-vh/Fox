//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : Source.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the SourceManager, SourceLoc and SourceRange
// classes.
//----------------------------------------------------------------------------//

#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <tuple>
#include "string_view.hpp"

namespace fox {
  class SourceLoc;
  // The FileID is an opaque object that packs a 16 bytes integer, representing a FileID
  class FileID {
    public:
      using IDTy = std::uint16_t;

      FileID();

      bool isValid() const;
      explicit operator bool() const;

      bool operator ==(const FileID other) const;
      bool operator !=(const FileID other) const;

      // For stl
      bool operator <(const FileID other) const;

      IDTy getRaw() const;

    protected:
      FileID(IDTy value);
      friend class SourceManager;

      IDTy get() const;
      void set(IDTy value);
      void markAsInvalid();

    private:
      IDTy value_;
  };

  // Small POD-like struct containing a human-readable source loc information.
  struct CompleteLoc {
    using LineTy = std::uint32_t;
    using ColTy = std::uint16_t;

    CompleteLoc(const std::string& fName, LineTy ln, ColTy col)
      : fileName(fName), line(ln), column(col) {

    }

    bool operator==(const CompleteLoc& other) const {
      return (fileName == other.fileName) 
        && (line == other.line) && (column == other.column);
    }

    std::string fileName;
    LineTy line;
    ColTy column;
  };

  // The SourceLoc is a lightweight wrapper around a FileID and an index
  // which, combined, represent the location of a character in the source code.
  // Note: this object expects a "byte" index, not an index in codepoints.
  class SourceLoc {
    public:
      using IndexTy = std::size_t;

      SourceLoc();
      explicit SourceLoc(FileID fid, IndexTy idx = 0);

      bool isValid() const;
      explicit operator bool() const; // ShortCut for isValid

      bool operator ==(const SourceLoc other) const;
      bool operator !=(const SourceLoc other) const;

      FileID getFileID() const;
      IndexTy getIndex() const;

    protected:
      friend class Parser;

      void increment();
      void decrement();
    private:
      FileID fid_;
      IndexTy idx_;
  };

  // The SourceRange is a wrapper around a SourceLoc and an offset, 
  // which combined represent a range (word, sentence, piece of code) 
  // in the source code.
  // Note: Like SourceLoc, the offset is expected to be absolute, not in CPs.
  class SourceRange {
    public:
      using OffsetTy = std::size_t;

      explicit SourceRange(SourceLoc sloc, OffsetTy offset = 0);
      explicit SourceRange(SourceLoc a, SourceLoc b);
      SourceRange();

      bool isValid() const;
      explicit operator bool() const; // Shortcut for isValid

      SourceLoc getBegin() const;
      OffsetTy getOffset() const;
      SourceLoc getEnd() const;
      bool isOnlyOneCharacter() const;

      FileID getFileID() const;
    private:
      SourceLoc sloc_;
      OffsetTy offset_;
  };

  // the SourceManager, which stores every source file and gives them a unique ID.
  // Files stored in/owned by the SourceManager are always immutable.
  class SourceManager {
    public:
      SourceManager() = default;

      struct SourceData {
        std::string fileName;
        std::string str;
        protected:
          using IndexTy = SourceLoc::IndexTy;
          using LineTy = CompleteLoc::LineTy;
          friend class SourceManager;

          SourceData(const std::string& name, const std::string& content)
              : fileName(name), str(content) {}

          mutable std::map<IndexTy, LineTy> lineTable_;
          mutable bool calculatedLineTable_ = false;
          // We cache the last search result here too.
          // The first element of the pair is the sourceloc that we
          // searched for, the second is the result we returned.
          mutable std::pair<SourceLoc, std::pair<IndexTy, LineTy>> 
            lastLTSearch_;
      };

      // Load a file in memory 
      FileID loadFromFile(const std::string& path);

      // Load a string in the SM. First arg is the string to load, 
      // the second is the name we should give to the file.
      FileID loadFromString(const std::string& str, 
        const std::string& name = "in-memory");

      // Returns a pointer to the source string of a file.
      // The result is always valid.
      string_view getSourceStr(FileID fid) const;

      // Returns a pointer to the "SourceData" for a given File.
      // The result is always non null (guaranteed by an assertion)
      // The result will also always be constant as the data stored
      // by the SourceManager is immutable.
      const SourceData* getSourceData(FileID fid) const;

      // Returns the line number of a SourceLoc
      CompleteLoc::LineTy getLineNumber(SourceLoc loc) const;

      // Requests the human-readable location a SourceLoc points to.
      // This function will assert that the SourceLoc is valid;
      // This function accepts a SourceLoc that points right past the end of the file.
      // Any value greater than that will trigger an assertion ("out of range")
      CompleteLoc getCompleteLoc(SourceLoc sloc) const;

      // Checks if a SourceLoc is valid
      bool checkValid(SourceLoc sloc) const;
      
      // Checks if a File Exists
      bool checkExists(FileID file) const;

      // Returns the complete line of source code for a given SourceLoc
      // An optional argument (pointer) can be passed. If it is present, the function
      // will store the Index at which the line begins in this variable.
      string_view getSourceLine(SourceLoc loc, 
        SourceLoc::IndexTy* lineBeg = nullptr) const;

    private:
      FileID generateNewFileID() const;
      void calculateLineTable(const SourceData* data) const;

      std::pair<SourceLoc::IndexTy, CompleteLoc::LineTy>
      searchLineTable(const SourceData* data, const SourceLoc& loc) const;

      // Make it non copyable
      SourceManager(const SourceManager&) = delete;
      SourceManager& operator=(const SourceManager&) = delete;
      
      // Member variables
      std::map<FileID,SourceData> sources_;
      // We'll always cache the latest search to speed things up
      mutable std::pair<FileID, const SourceData*> lastSource_ 
        = {FileID(), nullptr}; 
  };
}
