//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : Source.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Common/Source.hpp"
#include "Fox/Common/StringManipulator.hpp"
#include "utfcpp/utf8.hpp"
#include <fstream>
#include <cassert>
#include <cctype>

#define INVALID_FILEID_VALUE 0

using namespace fox;

// FileID

FileID::FileID()
{
  markAsInvalid();
}

FileID::FileID(id_type value)
{
  set(value);
}

bool FileID::isValid() const
{
  return value_ != INVALID_FILEID_VALUE;
}

FileID::operator bool() const
{
  return isValid();
}

bool FileID::operator==(const FileID other) const
{
  return value_ == other.value_;
}

bool FileID::operator!=(const FileID other) const
{
  return !(*this == other);
}

bool FileID::operator <(const FileID other) const
{
  return (value_ < other.value_);
}

FileID::id_type FileID::getRaw() const
{
  return value_;
}

FileID::id_type FileID::get() const
{
  return value_;
}

void FileID::set(id_type value)
{
  value_ = value;
}

void FileID::markAsInvalid()
{
  value_ = INVALID_FILEID_VALUE;
}

// SourceManager
const std::string* SourceManager::getSourceForFID(FileID fid) const
{
  auto data = getStoredDataForFileID(fid);
  return &(data->str);
}

const SourceManager::StoredData* SourceManager::getStoredDataForFileID(FileID fid) const
{
  assert(fid.isValid() && "Invalid FileID");
  auto it = sources_.find(fid);
  assert((it != sources_.end()) && "Unknown entry");
  return &(it->second);
}

CompleteLoc SourceManager::getCompleteLocForSourceLoc(SourceLoc sloc) const
{
  const StoredData* fdata = getStoredDataForFileID(sloc.getFileID());

  auto idx = sloc.getIndex();
  assert((idx <= fdata->str.size()) && "SourceLoc is Out-of-Range");

  // if the SourceLoc points to a fictive location just past the end
  // of the source, remove the extra column to avoid out_of_range errors9
  bool isOutOfRange = (idx == fdata->str.size());
  if (isOutOfRange)
    idx--;

  CompleteLoc::col_type col = 1;
  CompleteLoc::line_type line = 1;

  auto entry = getLineTableEntryForLoc(fdata, sloc);
  bool exactMatch = (entry.first == idx);
  if (exactMatch)
  {
    line = entry.second;
    col = 1;
  }
  else
  {
    line = entry.second;
    auto str_beg = fdata->str.c_str(); // Pointer to the first character of the string
    auto raw_col = utf8::distance(str_beg + entry.first, str_beg + idx);
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

bool SourceManager::isSourceLocValid(SourceLoc sloc) const
{
  const StoredData* data = getStoredDataForFileID(sloc.getFileID());
  
  if (!data)
    return false;

  // Less-or-equal because it might be a SourceLoc 
  // that points right after the end of the buffer.
  return sloc.getIndex() <= data->str.size();
}

bool SourceManager::doesFileExists(FileID file) const
{
  return (bool)getStoredDataForFileID(file);
}

std::string SourceManager::getLineAtLoc(SourceLoc loc, SourceLoc::idx_type* lineBeg) const
{
  const StoredData* data = getStoredDataForFileID(loc.getFileID());
  auto pair = getLineTableEntryForLoc(data, loc);

  std::size_t k = pair.first;

  if (lineBeg)
    *lineBeg = k;

  std::size_t sz = data->str.size();

  std::string rtr;

  char ch = 0;
  for (; k < sz; k++)
  {
    ch = data->str[k];

    if (ch == '\n' || ch == '\r')
      break;
    rtr += ch;
  }

  return rtr;
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

std::pair<SourceLoc::idx_type, CompleteLoc::line_type>
SourceManager::getLineTableEntryForLoc(const StoredData* data, const SourceLoc& loc) const
{
  if (!data->hasCalculatedLineTable)
    calculateLineTable(data);

  // Search the map, and find if this is an exact match.
  auto it = data->lineTable.lower_bound(loc.getIndex());

  bool exactMatch = false;
  if(it != data->lineTable.end())
    exactMatch = (it->first == loc.getIndex());

  if (exactMatch)
    return *it;
  else if (it != data->lineTable.begin())
    return *(--it);
  return *it;
}

// SourceLoc
SourceLoc::SourceLoc() : fid_(FileID()), idx_(0)
{

}

SourceLoc::SourceLoc(FileID fid, idx_type idx):
  fid_(fid), idx_(idx)
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

bool SourceLoc::operator==(const SourceLoc other) const
{
  return (fid_ == other.fid_) && (idx_ == other.idx_);
}

bool SourceLoc::operator!=(const SourceLoc other) const
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
SourceRange::SourceRange(SourceLoc sloc, offset_type offset):
  sloc_(sloc), offset_(offset)
{

}

SourceRange::SourceRange(SourceLoc a, SourceLoc b)
{
  // a and b must belong to the same file in all cases!
  assert(a.getFileID() == b.getFileID() && "A and B are from different files");
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

SourceLoc SourceRange::getBegin() const
{
  return sloc_;
}

SourceRange::offset_type SourceRange::getOffset() const
{
  return offset_;
}

SourceLoc SourceRange::getEnd() const
{
  return SourceLoc(sloc_.getFileID(), sloc_.getIndex() + offset_);
}

bool SourceRange::isOnlyOneCharacter() const
{
  return (offset_ == 0);
}

FileID SourceRange::getFileID() const
{
  return sloc_.getFileID();
}