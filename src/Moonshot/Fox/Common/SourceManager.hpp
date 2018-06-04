////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : SourceManager.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file contains the SourceManager, SourceLoc and SourceRange
// classes.
////------------------------------------------------------////

// 1) Add getStats
// 2) modify the sourcemanager so it stores a struct instead of a string as the map data, the struct will contain the file name & file data
// 3) test, test, and test.

#pragma once

#include <cstdint>
#include <map>
#include <string>

namespace Moonshot
{
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

			type get() const;
			void set(const type& value);
			void markAsInvalid();
		private:
			type value_;
	};

	class SourceManager
	{
		public:
			// Load a file in memory 
			FileID loadFromFile(const std::string& path);

			// Load a string in memory
			FileID loadFromString(const std::string& str);

			// Returns a pointer to the string that the FileID points to, or nullptr if not found
			const std::string* getSourceForFID(const FileID& fid) const;

		private:
			// The context is our friend!
			friend class Context;

			// Private methods
			FileID generateNewFileID() const;

			// Private constructor, so only the Context can create a SourceManager.
			SourceManager() = default;

			// Make it non copyable
			SourceManager(const SourceManager&) = delete;
			SourceManager& operator=(const SourceManager&) = delete;

			// Member variables
			std::map<FileID, std::string> sources_;
	};
	
	// The SourceLoc is a lightweight wrapper around a FileID and an index
	// which, combined, represent the location of a character in the source code.
	// Note: this object expects an "absolute" index, not an index in "codepoints".
	class SourceLoc
	{
		public:
			typedef std::uint32_t idx_type;
		
			SourceLoc();
			SourceLoc(const FileID& fid, const idx_type& idx);

			operator bool() const;

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
			typedef std::uint16_t offset_type;

			SourceRange(const SourceLoc& sloc, const offset_type& offset);
			SourceRange(const SourceLoc& a, const SourceLoc& b);

			operator bool() const;

			SourceLoc getBeginSourceLoc() const;
			offset_type getOffset() const;
			SourceLoc makeEndSourceLoc() const;
		private:
			SourceLoc sloc_;
			offset_type offset_;

	};
}