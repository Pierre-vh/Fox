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
#include <tuple>

namespace fox {
  class SourceLoc;

  class SourceManager;

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
  //
  // It allows the client to retrieve the data behind a FileID, like
  // the name of the file or it's content. 
  //
  // As an implementation detail, accessing a File's data is pretty
  // cheap since files are stored in a vector, so accessing the data
  // of any file is just a bit of pointer arithmetic.
  //  (data[theID.getIndex()] = the source file)
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
      string_view getFileContent(FileID fid) const;

      // Returns a string_view of the Source file's name.
      // The FileID MUST be valid.
      string_view getFileName(FileID fid) const;

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
      string_view getLineAt(SourceLoc loc, 
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
