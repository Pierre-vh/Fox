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
      using id_type = std::uint16_t;

      FileID();

      bool isValid() const;
      explicit operator bool() const;

      bool operator ==(const FileID other) const;
      bool operator !=(const FileID other) const;

      // For stl
      bool operator <(const FileID other) const;

      id_type getRaw() const;

    protected:
      FileID(id_type value);
      friend class SourceManager;

      id_type get() const;
      void set(id_type value);
      void markAsInvalid();

    private:
      id_type value_;
  };

  // Small POD-like struct containing a human-readable source loc information.
  struct CompleteLoc {
    using line_type = std::uint32_t;
    using col_type = std::uint16_t;

    CompleteLoc(const std::string& fName, line_type ln, col_type col)
      : fileName(fName), line(ln), column(col) {

    }

    bool operator==(const CompleteLoc& other) const {
      return (fileName == other.fileName) && (line == other.line) && (column == other.column);
    }

    std::string fileName;
    line_type line;
    col_type column;
  };

  // The SourceLoc is a lightweight wrapper around a FileID and an index
  // which, combined, represent the location of a character in the source code.
  // Note: this object expects an "absolute" index, not an index in "codepoints".
  class SourceLoc {
    public:
      using idx_type = std::size_t;

      SourceLoc();
      explicit SourceLoc(FileID fid, idx_type idx = 0);

      bool isValid() const;
      explicit operator bool() const; // ShortCut for isValid

      bool operator ==(const SourceLoc other) const;
      bool operator !=(const SourceLoc other) const;

      FileID getFileID() const;
      idx_type getIndex() const;

    protected:
      friend class Parser;

      void increment();
      void decrement();
    private:
      FileID fid_;
      idx_type idx_;
  };

  // The SourceRange is a wrapper around a SourceLoc and an offset, which combined represent
  // a range (word, sentence, piece of code) in the source code.
  // Note: Like SourceLoc, the offset is expected to be absolute, not in CPs.
  class SourceRange {
    public:
      using offset_type = std::size_t;

      explicit SourceRange(SourceLoc sloc, offset_type offset = 0);
      explicit SourceRange(SourceLoc a, SourceLoc b);
      SourceRange();

      bool isValid() const;
      explicit operator bool() const; // Shortcut for isValid

      SourceLoc getBegin() const;
      offset_type getOffset() const;
      SourceLoc getEnd() const;
      bool isOnlyOneCharacter() const;

      FileID getFileID() const;
    private:
      SourceLoc sloc_;
      offset_type offset_;
  };

  // the SourceManager, which stores every source file and gives them a unique ID.
  class SourceManager {
    public:
      SourceManager() = default;

      struct SourceData {
        std::string fileName;
        std::string str;
        protected:
          friend class SourceManager;

          SourceData(const std::string& name, const std::string& content)
              : fileName(name), str(content) {}

          mutable std::map<SourceLoc::idx_type,CompleteLoc::line_type> lineTable;
          mutable bool hasCalculatedLineTable = false;
      };

      // Load a file in memory 
      FileID loadFromFile(const std::string& path);

      // Load a string in the SM. First arg is the string to load, 
      // the second is the name we should give to the file.
      FileID loadFromString(const std::string& str, const std::string& name = "in-memory");

      // Returns a pointer to the source string of a file.
      // The result is always valid.
      string_view getSourceStr(FileID fid) const;

      // Returns a pointer to the "SourceData" for a given File.
      // The result is always non null (guaranteed by an assertion)
      const SourceData* getSourceData(FileID fid) const;

      // Returns the line number of a SourceLoc
      CompleteLoc::line_type getLineNumber(SourceLoc loc) const;

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
      string_view getSourceLine(SourceLoc loc, SourceLoc::idx_type* lineBeg = nullptr) const;

    private:
      FileID generateNewFileID() const;
      void calculateLineTable(const SourceData* data) const;

      std::pair<SourceLoc::idx_type, CompleteLoc::line_type>
      getLineTableEntry(const SourceData* data, const SourceLoc& loc) const;

      // Make it non copyable
      SourceManager(const SourceManager&) = delete;
      SourceManager& operator=(const SourceManager&) = delete;
      
      // Member variables
      std::map<FileID,SourceData> sources_;
  };
}
