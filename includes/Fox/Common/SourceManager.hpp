//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : SourceManager.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the SourceManager and associated classes.
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
  /// Object representing a human-readable version of a SourceLoc.
  ///
  /// Note that this structure does not own the "fileName" string.
  struct CompleteLoc {
    using line_type = std::uint32_t;
    using col_type = std::uint16_t;

    CompleteLoc(string_view fName, line_type ln, col_type col);

    bool operator==(const CompleteLoc& other) const;
    bool operator!=(const CompleteLoc& other) const;

    /// \returns the CompleteLoc as a string formatted like this:
    ///  file:line:column  (if printFilename = true)
    ///  line:column       (if printFilename = false)
    std::string toString(bool printFilename = true) const;

    /// The name of the file
    const string_view fileName;
    /// The line number
    const line_type line;
    /// The column number
    const col_type column;
  };

  /// Object representing a human-readable version of a SourceRange.
  /// Note that this structure does not own the "fileName" string.
  struct CompleteRange {
    using line_type = CompleteLoc::line_type;
    using col_type = CompleteLoc::col_type;

    CompleteRange(string_view fName, line_type begLine, col_type begCol,
      line_type endLine, col_type endCol);

    bool operator==(const CompleteRange& other) const;
    bool operator!=(const CompleteRange& other) const;

    /// \returns the CompleteRange as a string formatted like this:
    ///  file:line:column-line:column (if printFilename = true)
    ///  line:column-line:column      (if printFilename = true)
    std::string toString(bool printFilename = true) const;

    /// The name of the file
    const string_view fileName;
    /// The line number of the beginning of the loc
    const line_type begLine;
    /// The column number of the beginning of the loc
    const col_type begColumn;
    /// The line number of the end of the loc
    const line_type endLine;
    /// The column number of the end of the loc
    const col_type endColumn;
  };

  /// The SourceManager manages source files. It reads
  /// them, stores them and assigns a unique FileID to each of them.
  /// It also offers multiple functionalities related to File and SourceLocs,
  /// such as converting them into human-readable representations
  ///
  /// As an implementation detail, accessing a File's data is pretty
  /// cheap since files are stored in a vector.
  class SourceManager {
    public:
      using line_type = CompleteLoc::line_type;
      using col_type = CompleteLoc::col_type;

      SourceManager() = default;

      /// Make this class non copyable
      SourceManager(const SourceManager&) = delete;
      SourceManager& operator=(const SourceManager&) = delete;

      /// Return enum for readFile
      enum class ReadFileResult : std::uint8_t {
        /// The file was successfully read and loaded in memory.
        /// Its FileID is the first element of the pair.
        Ok,
        /// The file couldn't be found.
        NotFound,
        /// The file had an invalid encoding. 
        /// (Currently, only ASCII and UTF8 are supported.)
        InvalidEncoding
      };

      /// Loads a file in memory. 
      ///  \returns a pair. The first element is the FileID, it'll evaluate
      ///  to false if the file was not loaded in memory.
      ///  The second element contains the result information.
      std::pair<FileID, ReadFileResult> readFile(string_view path);

      /// Loads (copies) a string in the SourceManager
      /// \param str the string to load
      /// \param name the name the string should have
      /// \returns the FileID assigned to the string.
      FileID loadFromString(string_view str,
                            string_view name);

      /// \returns a string_view of \p fid 's buffer.
      /// (the string is owned by the SourceManager)
      string_view getFileContent(FileID fid) const;

      /// \returns a string_view of \p fid 's file name.
      /// (the string is owned by the SourceManager)
      string_view getFileName(FileID fid) const;

      /// \returns the line number of \p loc
      line_type getLineNumber(SourceLoc loc) const;

      /// \returns a complete, presentable version of \p sloc
      CompleteLoc getCompleteLoc(SourceLoc sloc) const;

      /// \returns a complete, presentable version of \p range
      CompleteRange getCompleteRange(SourceRange range) const;

      /// \param loc the loc
      /// \param lineBeg If it is present, the function will store the SourceLoc
      /// of the first character of the line in that loc.
      /// \returns the complete line of code in which \p loc is located.
      string_view getLineAt(SourceLoc loc, 
        SourceLoc* lineBeg = nullptr) const;

      /// Increments \p loc by \p count codepoints.
      /// \returns the incremented version of \p loc
      /// NOTE: This may fail if you try to increment past the end.
      SourceLoc advance(SourceLoc loc, std::size_t count = 1) const;

      /// \returns the number of codepoints contained in the range.
      /// \verbatim
      /// e.g. Let's say that:
      ///  a = range.getBeginLoc()
      ///  b = range.getEndLoc()
      ///
      ///  and that the range represents "fooba" in "foobar"
      ///
      ///    foobar
      ///    ^   ^
      ///    a   b
      ///
      ///  then getLengthInCodepoints(a, b) returns 5, because there's
      ///  5 characters in this range: fooba.
      ///
      /// Note: this method considers fullwidth unicode characters
      ///       as 1 codepoint, even if they take 2 characters on screen.
      /// \endverbatim
      std::size_t getLengthInCodepoints(SourceRange range) const;

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

          friend class SourceManager;

          // This is the cached "line table", which is used to efficiently
          // calculate the line number of a SourceLoc.
          mutable std::map<IndexTy, line_type> lineTable_;

          // Flag indicating whether we have calculated the LineTable.
          mutable bool calculatedLineTable_ = false;

          // We cache the last line table search here.
          // The first element of the pair is the raw index of
          // the last SourceLoc that we
          // searched for, the second is the result we returned for
          // that search.
          //
          // TODO: Is this case common enough to warrant caching? Tests needed!
          mutable std::pair<IndexTy, std::pair<IndexTy, line_type>> 
            lastLTSearch_;
      };

      // Calculates the line and column number of a given position inside
      // the file/data's content.
      std::pair<line_type, col_type>
      calculateLineAndColumn(const Data* data, SourceLoc::IndexTy idx) const;

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

      // Searches the "line table" of a given Data, returning
      // the index at which the line begins and the line number
      // at the position "idx"
      std::pair<SourceLoc::IndexTy, line_type>
      searchLineTable(const Data* data, SourceLoc::IndexTy idx) const;

      // Inserts a new Data in the datas_ vector, returning it's FileID.
      FileID insertData(std::unique_ptr<Data> data);
      
      // Member variables
      SmallVector<std::unique_ptr<Data>, 4> datas_;
  };
  
  // Converts a SourceManager::ReadFileResult to a string.
  std::string toString(SourceManager::ReadFileResult status);
}
