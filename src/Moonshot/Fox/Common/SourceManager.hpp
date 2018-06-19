////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : SourceManager.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file contains the SourceManager, SourceLoc and SourceRange
// classes.
////------------------------------------------------------////

#pragma once

#include <cstdint>
#include <map>
#include <string>
#include "Moonshot/Fox/Common/Typedefs.hpp"

namespace Moonshot
{
	class SourceLoc;
	// The FileID struct is a wrapper around a 16 bytes integer, representing a FileID
	struct FileID
	{
		public:
			typedef std::uint16_t type;

			FileID();
			FileID(const type& value);

			operator bool() const;
			bool operator ==(const FileID& other) const;
			bool operator !=(const FileID& other) const;
			// For comparisons
			bool operator <(const FileID& other) const;

			type get() const;
			void set(const type& value);
			void markAsInvalid();

		private:
			type value_;
	};

	// Small struct containing a human-readable source loc information.
	struct CompleteLoc
	{
		CompleteLoc(const std::string& fName, const std::uint32_t& ln, const std::uint16_t& col, const std::uint16_t& chidx, const CharType& ch)
			: fileName(fName), line(ln), column(col), character(chidx), value(ch)
		{

		}

		std::string fileName;
		std::uint32_t line;
		std::uint16_t column;
		std::uint16_t character;
		CharType value;
	};

	// the SourceManager, which stores every source file and gives them a unique ID.
	class SourceManager
	{
		public:
			struct StoredData
			{
				StoredData(const std::string& name, const std::string& content) : fileName(name), str(content)
				{

				}

				std::string fileName;
				std::string str;
			};

			// Load a file in memory 
			FileID loadFromFile(const std::string& path);

			// Load a string in memory
			FileID loadFromString(const std::string& str);

			// Returns a pointer to the string that the FileID points to, or nullptr if not found
			const std::string* getSourceForFID(const FileID& fid) const;
			const StoredData*  getStoredDataForFileID(const FileID& fid) const;
			CompleteLoc getCompleteLocForSourceLoc(const SourceLoc& sloc) const;

		private:

			// The context is our friend!
			friend class Context;

			// Private methods
			FileID generateNewFileID() const;
			CharType extractCharFromStr(const std::string* str, const std::size_t& idx) const;

			// Private constructor, so only the Context can create a SourceManager.
			SourceManager() = default;

			// Make it non copyable
			SourceManager(const SourceManager&) = delete;
			SourceManager& operator=(const SourceManager&) = delete;
			
			// Member variables
			std::map<FileID,StoredData> sources_;
	};
	
	// The SourceLoc is a lightweight wrapper around a FileID and an index
	// which, combined, represent the location of a character in the source code.
	// Note: this object expects an "absolute" index, not an index in "codepoints".
	class SourceLoc
	{
		public:
			typedef std::size_t idx_type;
		
			SourceLoc();
			SourceLoc(const FileID& fid, const idx_type& idx);

			bool isValid() const;
			operator bool() const; // ShortCut for isValid

			bool operator ==(const SourceLoc& other) const;
			bool operator !=(const SourceLoc& other) const;

			FileID getFileID() const;
			idx_type getIndex() const;
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

			SourceRange(const SourceLoc& sloc, const offset_type& offset = 0);
			SourceRange(const SourceLoc& a, const SourceLoc& b);
			SourceRange();

			bool isValid() const;
			operator bool() const; // Shortcut for isValid

			SourceLoc getBeginSourceLoc() const;
			offset_type getOffset() const;
			SourceLoc makeEndSourceLoc() const;
			bool isOnlyOneCharacter() const;
		private:
			SourceLoc sloc_;
			offset_type offset_;

	};
}