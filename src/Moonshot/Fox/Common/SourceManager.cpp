////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : SourceManager.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "SourceManager.hpp"

#include <fstream>
#include <cassert>

#define INVALID_FILEID_VALUE 0

using namespace Moonshot;

// FileID

FileID::FileID()
{
	markAsInvalid();
}

FileID::FileID(const type & value)
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

FileID::type FileID::get() const
{
	return value_;
}

void FileID::set(const type & value)
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
	auto it = sources_.lower_bound(fid);
	if (it != sources_.end() && !(sources_.key_comp()(fid, it->first)))
		return &(it->second);
	return nullptr;
}


FileID SourceManager::loadFromFile(const std::string & path)
{
	std::ifstream in(path, std::ios::binary);
	if (in)
	{
		auto pair = sources_.insert(std::pair<FileID, std::string>(generateNewFileID(), (std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()))));
		return (pair.first)->first;
	}
	return FileID();
}

FileID SourceManager::loadFromString(const std::string & str)
{
	auto pair = sources_.insert(std::pair<FileID,std::string>(generateNewFileID(),str));
	return (pair.first)->first;
}

FileID SourceManager::generateNewFileID() const
{
	// The newly generated fileID is always the size of source_ +1, since 0 is the invalid value for FileIDs
	FileID::type id = static_cast<FileID::type>(sources_.size() + 1);
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

SourceLoc::operator bool() const
{
	return (bool)fid_;
}

FileID SourceLoc::getFileID() const
{
	return fid_;
}

SourceLoc::idx_type SourceLoc::getIndex() const
{
	return idx_;
}

SourceRange::SourceRange(const SourceLoc& sloc, const offset_type& offset) : sloc_(sloc), offset_(offset)
{

}

SourceRange::SourceRange(const SourceLoc& a, const SourceLoc& b)
{
	// a and b must belong to the same file in all cases!
	assert(a.getFileID() == b.getFileID());
	if (a.getIndex() <= b.getIndex())
	{
		// a is the first sloc
		sloc_ = a;
		offset_ = static_cast<offset_type>(b.getIndex() - a.getIndex());
	}
	else 
	{
		// b is the first sloc
		sloc_ = b;
		offset_ = static_cast<offset_type>(a.getIndex() - b.getIndex());
	}
}

SourceRange::operator bool() const
{
	return (bool)sloc_;
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
