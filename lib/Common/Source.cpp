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

FileID::FileID() {
  markAsInvalid();
}

FileID::FileID(IDTy value) {
  set(value);
}

bool FileID::isValid() const {
  return value_ != INVALID_FILEID_VALUE;
}

FileID::operator bool() const {
  return isValid();
}

bool FileID::operator==(const FileID other) const {
  return value_ == other.value_;
}

bool FileID::operator!=(const FileID other) const {
  return !(*this == other);
}

bool FileID::operator <(const FileID other) const {
  return (value_ < other.value_);
}

FileID::IDTy FileID::getRaw() const {
  return value_;
}

FileID::IDTy FileID::get() const {
  return value_;
}

void FileID::set(IDTy value) {
  value_ = value;
}

void FileID::markAsInvalid() {
  value_ = INVALID_FILEID_VALUE;
}

// SourceManager
string_view SourceManager::getSourceStr(FileID fid) const {
  auto data = getSourceData(fid);
  return data->str;
}

const SourceManager::SourceData*
SourceManager::getSourceData(FileID fid) const {
  assert(fid.isValid() && "Invalid FileID");

  // First, check in the cache
  if (lastSource_.first == fid)
    return lastSource_.second;  
  
  // Map search required
  auto it = sources_.find(fid);
  assert((it != sources_.end()) && "Unknown entry");
  const SourceData* ptr = &(it->second);
  
  // Cache the result & return.
  lastSource_ = {fid, ptr};
  return ptr;
}

CompleteLoc::LineTy SourceManager::getLineNumber(SourceLoc loc) const {
  auto result = getLineTableEntry(
    getSourceData(loc.getFileID()), loc);
  return result.second;
}

CompleteLoc SourceManager::getCompleteLoc(SourceLoc sloc) const {
  const SourceData* fdata = getSourceData(sloc.getFileID());

  auto idx = sloc.getIndex();
  assert((idx <= fdata->str.size()) && "SourceLoc is Out-of-Range");

  // if the SourceLoc points to a fictive location just past the end
  // of the source, remove the extra column to avoid out_of_range errors9
  bool isOutOfRange = (idx == fdata->str.size());
  if (isOutOfRange)
    idx--;

  CompleteLoc::ColTy col = 1;
  CompleteLoc::LineTy line = 1;

  auto entry = getLineTableEntry(fdata, sloc);
  bool exactMatch = (entry.first == idx);
  if (exactMatch) {
    line = entry.second;
    col = 1;
  }
  else {
    line = entry.second;
    auto str_beg = fdata->str.c_str(); // Pointer to the first character of the string
    auto raw_col = utf8::distance(str_beg + entry.first, str_beg + idx);
    col = static_cast<CompleteLoc::ColTy>(raw_col+1);
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

bool SourceManager::checkValid(SourceLoc sloc) const {
  const SourceData* data = getSourceData(sloc.getFileID());
  
  if (!data)
    return false;

  // Less-or-equal because it might be a SourceLoc 
  // that points right after the end of the buffer.
  return sloc.getIndex() <= data->str.size();
}

bool SourceManager::checkExists(FileID file) const {
  return (bool)getSourceData(file);
}

string_view SourceManager::getSourceLine(SourceLoc loc, SourceLoc::IndexTy* lineBeg) const {
  const SourceData* data = getSourceData(loc.getFileID());
  string_view source = data->str;

  auto pair = getLineTableEntry(data, loc);

  std::size_t beg = pair.first, end = beg;

  if (lineBeg) (*lineBeg) = beg;

  for (; end < source.size(); end++) {
    if (source[end] == '\n' || source[end] == '\r')
      break;
  }

  return source.substr(beg, end-beg);
}

FileID SourceManager::loadFromFile(const std::string & path) {
  std::ifstream in(path, std::ios::binary);
  if (in) {
    auto pair = sources_.insert(std::pair<FileID,SourceData>(generateNewFileID(),
      SourceData(
        path,
        (std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>())))
      )
    );
    return (pair.first)->first;
  }
  return FileID();
}

FileID SourceManager::loadFromString(const std::string& str, const std::string& name) {
  auto pair = sources_.insert(std::pair<FileID,SourceData>(generateNewFileID(),SourceData(name,str)));
  return (pair.first)->first;
}

FileID SourceManager::generateNewFileID() const {
  // The newly generated fileID is always the size of source_ +1, since 0 is the invalid value for FileIDs
  FileID::IDTy id = static_cast<FileID::IDTy>(sources_.size() + 1);
  assert(id != INVALID_FILEID_VALUE);
  return id;
}

void SourceManager::calculateLineTable(const SourceData* data) const {
  if(data->hasCalculatedLineTable)
    data->lineTable.clear();
  
  std::size_t size = data->str.size();
  CompleteLoc::LineTy line = 1;
  // Mark the index 0 as first line.
  data->lineTable[0] = 1;
  line++;
  for (std::size_t idx = 0; idx < size; idx++) {
    // Supported line endings : \r\n, \n
    // Just need to add +1 to the index in both cases to mark the beginning
    // of the line as the first character after \n
    if (data->str[idx] == '\n') {
      data->lineTable[idx+1] = line;
      line++;
    }
  }

  data->hasCalculatedLineTable = true;
}

std::pair<SourceLoc::IndexTy, CompleteLoc::LineTy>
SourceManager::getLineTableEntry(const SourceData* data, const SourceLoc& loc) const {
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
SourceLoc::SourceLoc() : fid_(FileID()), idx_(0) {

}

SourceLoc::SourceLoc(FileID fid, IndexTy idx):
  fid_(fid), idx_(idx) {
}

bool SourceLoc::isValid() const {
  return (bool)fid_;
}

SourceLoc::operator bool() const {
  return isValid();
}

bool SourceLoc::operator==(const SourceLoc other) const {
  return (fid_ == other.fid_) && (idx_ == other.idx_);
}

bool SourceLoc::operator!=(const SourceLoc other) const {
  return !(*this == other);
}

FileID SourceLoc::getFileID() const {
  return fid_;
}

SourceLoc::IndexTy SourceLoc::getIndex() const {
  return idx_;
}

void SourceLoc::increment() {
  idx_++;
}

void SourceLoc::decrement() {
  idx_--;
}

// SourceRange
SourceRange::SourceRange(SourceLoc sloc, OffsetTy offset):
  sloc_(sloc), offset_(offset) {

}

SourceRange::SourceRange(SourceLoc a, SourceLoc b) {
  // a and b must belong to the same file in all cases!
  assert(a.getFileID() == b.getFileID() && "A and B are from different files");
  if (a.getIndex() < b.getIndex()) {
    // a is the first sloc
    sloc_ = a;
    offset_ = static_cast<OffsetTy>(b.getIndex() - a.getIndex());
  }
  else if (a.getIndex() > b.getIndex()) {
    // b is the first sloc
    sloc_ = b;
    offset_ = static_cast<OffsetTy>(a.getIndex() - b.getIndex());
  }
  else  {
    // a == b
    sloc_ = a;
    offset_ = 0;
  }
}

SourceRange::SourceRange() : sloc_(SourceLoc()), offset_(0) {
  
}

bool SourceRange::isValid() const {
  return (bool)sloc_;
}

SourceRange::operator bool() const {
  return isValid();
}

SourceLoc SourceRange::getBegin() const {
  return sloc_;
}

SourceRange::OffsetTy SourceRange::getOffset() const {
  return offset_;
}

SourceLoc SourceRange::getEnd() const {
  return SourceLoc(sloc_.getFileID(), sloc_.getIndex() + offset_);
}

bool SourceRange::isOnlyOneCharacter() const {
  return (offset_ == 0);
}

FileID SourceRange::getFileID() const {
  return sloc_.getFileID();
}
