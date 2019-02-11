//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : SourceManager.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the SourceManager class
//----------------------------------------------------------------------------//

#pragma once

#include "string_view.hpp"
#include "LLVM.hpp"
#include "SourceLoc.hpp"
#include "llvm/ADT/SmallVector.h"
#include <memory>
#include <string>
#include <map>

namespace fox {
  // Object containing a human-readable version of a SourceLoc.
  //
  // Note that this structure does not own the "fileName" string.
  struct CompleteLoc {
    using line_type = std::uint32_t;
    using col_type = std::uint16_t;

    CompleteLoc(string_view fName, line_type ln, col_type col);

    bool operator==(const CompleteLoc& other) const;
    bool operator!=(const CompleteLoc& other) const;

    std::string toString(bool printFilename = true) const;

    const string_view fileName;
    const line_type line;
    const col_type column;
  };

  // Object containing a human-readable version of a SourceRange.
  struct CompleteRange {
    using line_type = CompleteLoc::line_type;
    using col_type = CompleteLoc::col_type;

    CompleteRange(string_view fName, line_type begLine, col_type begCol,
      line_type endLine, col_type endCol);

    bool operator==(const CompleteRange& other) const;
    bool operator!=(const CompleteRange& other) const;

    std::string toString(bool printFilename = true) const;

    const string_view fileName;
    const line_type begLine;
    const col_type begColumn;
    const line_type endLine;
    const col_type endColumn;
  };

  // The SourceManager, which manages source files. It reads
  // them, stores them and gives them a unique FileID.
  //
  // It also allows the client to retrieve the data behind a FileID,
  // SourceLoc or SourceRange.
  //
  // As an implementation detail, accessing a File's data is pretty
  // cheap since files are stored in a vector, so accessing the data
  // of any file is just a bit of pointer arithmetic.
  //  (data[theID.getIndex()] = the source file)
  class SourceManager {
    public:
      using line_type = CompleteLoc::line_type;
      using col_type = CompleteLoc::col_type;

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
                            string_view name);

      // Returns a string_view of the Source file's content.
      // The FileID MUST be valid.
      string_view getFileContent(FileID fid) const;

      // Returns a string_view of the Source file's name.
      // The FileID MUST be valid.
      string_view getFileName(FileID fid) const;

      // Returns the line number of a SourceLoc
      line_type getLineNumber(SourceLoc loc) const;

      // Requests the "complete" version of a SourceLoc, which
      // is presentable to the user.
      CompleteLoc getCompleteLoc(SourceLoc sloc) const;

      // Requests the "complete" version of a SourceLoc, which
      // is presentable to the user.
      CompleteRange getCompleteRange(SourceRange range) const;

      // Returns the complete line of source code for a given SourceLoc
      // An optional argument (pointer) can be passed. If it is present,
      // the function will store the Index at which the line begins in 
      // this variable.
      string_view getLineAt(SourceLoc loc, 
        SourceLoc::index_type* lineBeg = nullptr) const;

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
          using index_type = SourceLoc::index_type;

          friend class SourceManager;

          // This is the cached "line table", which is used to efficiently
          // calculate the line number of a SourceLoc.
          mutable std::map<index_type, line_type> lineTable_;

          // Flag indicating whether we have calculated the LineTable.
          mutable bool calculatedLineTable_ = false;

          // We cache the last line table search here.
          // The first element of the pair is the raw index of
          // the last SourceLoc that we
          // searched for, the second is the result we returned for
          // that search.
          //
          // TODO: Is this case common enough to warrant caching? Tests needed!
          mutable std::pair<index_type, std::pair<index_type, line_type>> 
            lastLTSearch_;
      };

      // Calculates the line and column number of a given position inside
      // the file/data's content.
      std::pair<line_type, col_type>
      calculateLineAndColumn(const Data* data, SourceLoc::index_type idx) const;

      // Returns a pointer to the "Data" for a given File.
      // The result is always non null (guaranteed by an assertion)
      // The result will also always be constant because the data stored
      // is immutable.
      const Data* getData(FileID fid) const;

      // Calculates the "line table" of a given Data.
      void calculateLineTable(const Data* data) const;

      // Checks if a SourceLoc's index refers to a valid position
      // (or a past-the-end position) in the data.
      bool isIndexValid(const Data* data, SourceLoc::index_type idx) const;

      // Searches the "line table" of a given Data, returning
      // the index at which the line begins and the line number
      // at the position "idx"
      std::pair<SourceLoc::index_type, line_type>
      searchLineTable(const Data* data, SourceLoc::index_type idx) const;

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
