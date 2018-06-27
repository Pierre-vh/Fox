////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : SourceManager.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "SourceManager.hpp"
#include "Fox/Common/StringManipulator.hpp"

#include <fstream>
#include <cassert>

#define INVALID_FILEID_VALUE 0

using namespace fox;

// FileID

FileID::FileID()
{
	markAsInvalid();
}

FileID::FileID(const id_type& value)
{
	set(value);
}

FileID::operator bool() const
{
	return value_ != INVALID_FILEID_VALUE;
}

bool FileID::operator==(const FileID & other) const
{
	return value_ == other.value_;
}

bool FileID::operator!=(const FileID & other) const
{
	return !(*this == other);
}

bool FileID::operator <(const FileID& other) const
{
	return (value_ < other.value_);
}

FileID::id_type FileID::get() const
{
	return value_;
}

void FileID::set(const id_type& value)
{
	value_ = value;
}

void FileID::markAsInvalid()
{
	value_ = INVALID_FILEID_VALUE;
}

// SourceManager
const std::string* SourceManager::getSourceForFID(const FileID& fid) const
{
	if (auto data = getStoredDataForFileID(fid))
		return &(data->str);
	return nullptr;
}

const SourceManager::StoredData * SourceManager::getStoredDataForFileID(const FileID & fid) const
{
	auto it = sources_.lower_bound(fid);
	if (it != sources_.end() && !(sources_.key_comp()(fid, it->first)))
		return &(it->second);
	return nullptr;
}

CompleteLoc SourceManager::getCompleteLocForSourceLoc(const SourceLoc& sloc) const
{
	// ToDo: Optimize this by caching a line table. Also add support for \n\r and \r\n line endings.

	const StoredData* fdata = getStoredDataForFileID(sloc.getFileID());
	assert(fdata && "Entry does not exists?");

	auto idx = sloc.getIndex();
	assert((idx <= fdata->str.size()) && "SourceLoc is Out-of-Range");

	// Check if this SourceLoc is right past the end 
	bool isOutOfRange = (idx == fdata->str.size());

	// Remove the extra column to avoid going out of range on the string
	if (isOutOfRange)
		idx--;

	// Compute
	std::uint32_t line = 1;
	std::uint16_t column = 1;
	for (SourceLoc::idx_type k = 0; k < idx; k++)
	{
		switch (fdata->str[k])
		{
			// ToDo: Does this even work correctly? 
			// Will check once I implement the line table optimization.
			case '\r':
			case '\n':
				column = 1;
				line++;
				break;
			default:
				column++;
				break;
		}
	}

	// Add back the extra column
	if (isOutOfRange)
		column++;

	return CompleteLoc(
		fdata->fileName,
		line,
		column
	);
}

bool SourceManager::tryIncrementSourceLoc(SourceLoc& sloc, bool* incrementedPastTheEnd)
{
	const StoredData* data = getStoredDataForFileID(sloc.getFileID());

	assert(data && "Invalid SourceLoc");

	const auto dataSize = data->str.size();
	if (sloc.getIndex() < dataSize)
	{
		sloc.idx_++;

		if (incrementedPastTheEnd)
			*incrementedPastTheEnd = (sloc.getIndex() == dataSize);

		return true;
	}
	return false;
}

bool SourceManager::isSourceLocValid(const SourceLoc & sloc) const
{
	const StoredData* data = getStoredDataForFileID(sloc.getFileID());
	
	if (!data)
		return false;

	// Less-or-equal because it might be a SourceLoc 
	// that points right after the end of the buffer.
	return sloc.getIndex() <= data->str.size();
}

bool SourceManager::doesFileExists(const FileID & file) const
{
	return (bool)getStoredDataForFileID(file);
}

FileID SourceManager::loadFromFile(const std::string & path)
{
	std::ifstream in(path, std::ios::binary);
	if (in)
	{
		auto pair = sources_.insert(std::pair<FileID,StoredData>(generateNewFileID(),
			StoredData(
				path,
				(std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>())))
			)
		);
		return (pair.first)->first;
	}
	return FileID();
}

FileID SourceManager::loadFromString(const std::string& str, const std::string& name)
{
	auto pair = sources_.insert(std::pair<FileID,StoredData>(generateNewFileID(),StoredData(name,str)));
	return (pair.first)->first;
}

FileID SourceManager::generateNewFileID() const
{
	// The newly generated fileID is always the size of source_ +1, since 0 is the invalid value for FileIDs
	FileID::id_type id = static_cast<FileID::id_type>(sources_.size() + 1);
	assert(id != INVALID_FILEID_VALUE);
	return id;
}

// SourceLoc
SourceLoc::SourceLoc() : fid_(FileID()), idx_(0)
{

}

SourceLoc::SourceLoc(const FileID & fid, const idx_type & idx) : fid_(fid), idx_(idx)
{
}

bool SourceLoc::isValid() const
{
	return (bool)fid_;
}

SourceLoc::operator bool() const
{
	return isValid();
}

bool SourceLoc::operator==(const SourceLoc& other) const
{
	return (fid_ == other.fid_) && (idx_ == other.idx_);
}

bool SourceLoc::operator!=(const SourceLoc& other) const
{
	return !(*this == other);
}

FileID SourceLoc::getFileID() const
{
	return fid_;
}

SourceLoc::idx_type SourceLoc::getIndex() const
{
	return idx_;
}


// SourceRange
SourceRange::SourceRange(const SourceLoc& sloc, const offset_type& offset) : sloc_(sloc), offset_(offset)
{

}

SourceRange::SourceRange(const SourceLoc& a, const SourceLoc& b)
{
	// a and b must belong to the same file in all cases!
	assert(a.getFileID() == b.getFileID());
	if (a.getIndex() < b.getIndex())
	{
		// a is the first sloc
		sloc_ = a;
		offset_ = static_cast<offset_type>(b.getIndex() - a.getIndex());
	}
	else if (a.getIndex() > b.getIndex())
	{
		// b is the first sloc
		sloc_ = b;
		offset_ = static_cast<offset_type>(a.getIndex() - b.getIndex());
	}
	else 
	{
		// a == b
		sloc_ = a;
		offset_ = 0;
	}
}

SourceRange::SourceRange() : sloc_(SourceLoc()), offset_(0)
{
	
}

bool SourceRange::isValid() const
{
	return (bool)sloc_;
}

SourceRange::operator bool() const
{
	return isValid();
}

SourceLoc SourceRange::getBeginSourceLoc() const
{
	return sloc_;
}

SourceRange::offset_type SourceRange::getOffset() const
{
	return offset_;
}

SourceLoc SourceRange::makeEndSourceLoc() const
{
	return SourceLoc(sloc_.getFileID(), sloc_.getIndex() + offset_);
}

bool SourceRange::isOnlyOneCharacter() const
{
	return (offset_ == 0);
}