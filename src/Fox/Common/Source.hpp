////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Source.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file contains the SourceManager, SourceLoc and SourceRange
// classes.
////------------------------------------------------------////

#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <tuple>

namespace fox
{
	class SourceLoc;
	// The FileID is an opaque object that packs a 16 bytes integer, representing a FileID
	class FileID
	{
		public:
			typedef std::uint16_t id_type;

			FileID();

			bool isValid() const;
			explicit operator bool() const;

			bool operator ==(const FileID& other) const;
			bool operator !=(const FileID& other) const;
			// For comparisons
			bool operator <(const FileID& other) const;
		protected:
			FileID(const id_type& value);
			friend class SourceManager;

			id_type get() const;
			void set(const id_type& value);
			void markAsInvalid();

		private:
			id_type value_;
	};

	// Small POD-like struct containing a human-readable source loc information.
	struct CompleteLoc
	{
		typedef std::uint32_t line_type;
		typedef std::uint16_t col_type;

		CompleteLoc(const std::string& fName, line_type ln, col_type col)
			: fileName(fName), line(ln), column(col)
		{

		}

		bool operator==(const CompleteLoc& other) const
		{
			return (fileName == other.fileName) && (line == other.line) && (column == other.column);
		}

		std::string fileName;
		line_type line;
		col_type column;
	};

	// The SourceLoc is a lightweight wrapper around a FileID and an index
	// which, combined, represent the location of a character in the source code.
	// Note: this object expects an "absolute" index, not an index in "codepoints".
	class SourceLoc
	{
		public:
			typedef std::size_t idx_type;

			SourceLoc();
			explicit SourceLoc(const FileID& fid, idx_type idx = 0);

			bool isValid() const;
			explicit operator bool() const; // ShortCut for isValid

			bool operator ==(const SourceLoc& other) const;
			bool operator !=(const SourceLoc& other) const;

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
	class SourceRange
	{
		public:
			typedef std::size_t offset_type;

			explicit SourceRange(const SourceLoc& sloc, offset_type offset = 0);
			explicit SourceRange(const SourceLoc& a, const SourceLoc& b);
			SourceRange();

			bool isValid() const;
			explicit operator bool() const; // Shortcut for isValid

			SourceLoc getBegin() const;
			offset_type getOffset() const;
			SourceLoc getEnd() const;
			bool isOnlyOneCharacter() const;
		private:
			SourceLoc sloc_;
			offset_type offset_;
	};

	// the SourceManager, which stores every source file and gives them a unique ID.
	class SourceManager
	{
		public:
			SourceManager() = default;

			struct StoredData
			{
				public:
					StoredData(const std::string& name, const std::string& content) : fileName(name), str(content)
					{

					}

					std::string fileName;
					std::string str;
				protected:
					friend class SourceManager;
					mutable std::map<SourceLoc::idx_type,CompleteLoc::line_type> lineTable;
					mutable bool hasCalculatedLineTable = false;
			};

			// Load a file in memory 
			FileID loadFromFile(const std::string& path);

			// Load a string in the SM. First arg is the string to load, the second is the name we should give
			// to the file.
			FileID loadFromString(const std::string& str, const std::string& name = "in-memory");

			// Returns a pointer to the source string of a file.
			// The result is always non null.
			const std::string* getSourceForFID(const FileID& fid) const;

			// Returns a pointer to the stored data that the FileID points to.
			// The result is always non null.
			const StoredData*  getStoredDataForFileID(const FileID& fid) const;

			// Requests the human-readable location a SourceLoc points to.
			// This function will assert that the SourceLoc is valid;
			// This function accepts a SourceLoc that points right past the end of the file.
			// Any value greater than that will trigger an assertion ("out of range")
			CompleteLoc getCompleteLocForSourceLoc(const SourceLoc& sloc) const;

			// Check if a SourceLoc is valid
			bool isSourceLocValid(const SourceLoc& sloc) const;
			
			// Check if a File Exists
			bool doesFileExists(const FileID& file) const;

			// Returns the complete line of source code for a given SourceLoc
			// An optional argument (pointer) can be passed. If it is present, the function
			// will store the Index at which the line begins in this variable.
			std::string getLineAtLoc(const SourceLoc& loc, SourceLoc::idx_type* lineBeg = nullptr) const;
		private:
			FileID generateNewFileID() const;
			void calculateLineTable(const StoredData* data) const;

			std::pair<SourceLoc::idx_type, CompleteLoc::line_type>
			getLineTableEntryForLoc(const StoredData* data, const SourceLoc& loc) const;

			// Make it non copyable
			SourceManager(const SourceManager&) = delete;
			SourceManager& operator=(const SourceManager&) = delete;
			
			// Member variables
			std::map<FileID,StoredData> sources_;
	};
}