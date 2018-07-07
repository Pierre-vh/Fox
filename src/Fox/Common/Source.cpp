////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Source.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Source.hpp"
#include "Fox/Common/StringManipulator.hpp"
#include "utfcpp/utf8.hpp"
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
	const StoredData* fdata = getStoredDataForFileID(sloc.getFileID());
	assert(fdata && "Entry does not exists?");

	auto idx = sloc.getIndex();
	assert((idx <= fdata->str.size()) && "SourceLoc is Out-of-Range");

	// if the SourceLoc points to a fictive location just past the end
	// of the source, remove the extra column to avoid out_of_range errors9
	bool isOutOfRange = (idx == fdata->str.size());
	if (isOutOfRange)
		idx--;

	// Calculate the line table if needed
	if (!fdata->hasCalculatedLineTable)
		calculateLineTable(fdata);

	CompleteLoc::line_type line = 0;
	CompleteLoc::col_type col = 0;

	// Search the map, and find if this is an exact match.
	auto it = fdata->lineTable.lower_bound(idx);
	bool exactMatch = false;
	if (it != fdata->lineTable.end())
		exactMatch = (it->first == idx);

	if (exactMatch)
	{
		// If it's an exact match, just use the it->second as the line
		// and we don't need to calculate the column 
		line = it->second;
		col = 1;
	}
	else
	{
		// Since lower_bound give us an iterator to the element 
		// above what we're looking for, decrement it if possible
		if (it != fdata->lineTable.begin())
			it--;

		// get the line
		line = it->second;

		// Use the utf8 library to calculate the distance in codepoints
		// between the beginning of the line and the index we're looking for.
		auto str_beg = fdata->str.c_str(); // Pointer to the first character of the string
		auto raw_col = utf8::distance(str_beg + (it->first), str_beg + idx);

		// Avoid compiler errors
		col = static_cast<CompleteLoc::col_type>(raw_col+1);
	}

	// Add back the extra column if needed
	if (isOutOfRange)
		col++;

	return CompleteLoc(
		fdata->fileName,
		line,
		col
	);
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

void SourceManager::calculateLineTable(const StoredData* data) const
{
	if(data->hasCalculatedLineTable)
		data->lineTable.clear();
	
	std::size_t size = data->str.size();
	CompleteLoc::line_type line = 1;
	// Mark the index 0 as first line.
	data->lineTable[0] = 1;
	line++;
	for (std::size_t idx = 0; idx < size; idx++)
	{
		// Supported line endings : \r\n, \n
		// Just need to add +1 to the index in both cases to mark the beginning
		// of the line as the first character after \n
		if (data->str[idx] == '\n')
		{
			data->lineTable[idx+1] = line;
			line++;
		}
	}

	data->hasCalculatedLineTable = true;
}

// SourceLoc
SourceLoc::SourceLoc() : fid_(FileID()), idx_(0)
{

}

SourceLoc::SourceLoc(const FileID & fid, idx_type idx) : fid_(fid), idx_(idx)
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

void SourceLoc::increment()
{
	idx_++;
}

void SourceLoc::decrement()
{
	idx_--;
}

// SourceRange
SourceRange::SourceRange(const SourceLoc& sloc, offset_type offset) : sloc_(sloc), offset_(offset)
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