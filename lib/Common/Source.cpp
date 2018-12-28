#include "..\..\includes\Fox\Common\Source.hpp"
#include "..\..\includes\Fox\Common\Source.hpp"
#include "..\..\includes\Fox\Common\Source.hpp"
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
#include <sstream>
#include <cctype>
#include <iostream>

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
  auto result = searchLineTable(
    getSourceData(loc.getFileID()), loc);
  return result.second;
}

SourceRange SourceManager::getRangeOfFile(FileID file) const {
  using OffTy = SourceRange::OffsetTy;
  
  // Begin SourceLoc is always (file, 0)
  SourceLoc begin(file, 0);

  // Calculate end
  const SourceData* data = getSourceData(file);
  std::size_t size = data->str.size();

  // Check that the size isn't too big, just to be sure.
  assert(size < std::numeric_limits<OffTy>::max() &&
    "Can't create a file-wide SourceRange for this file: file is too large!");
  
  // Return the SourceRange
  return SourceRange(begin, static_cast<OffTy>(size));
}

CompleteLoc SourceManager::getCompleteLoc(SourceLoc sloc) const {
  const SourceData* fdata = getSourceData(sloc.getFileID());

  auto idx = sloc.getIndex();
  assert((idx <= fdata->str.size()) && "SourceLoc is Out-of-Range");

  // if the SourceLoc points to a fictive location just past the end
  // of the source, remove the extra column to avoid out_of_range errors
  bool isOutOfRange = (idx == fdata->str.size());
  if (isOutOfRange)
    idx--;

  CompleteLoc::ColTy col = 0;
  CompleteLoc::LineTy line = 0;

  auto entry = searchLineTable(fdata, sloc);
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

string_view 
SourceManager::getSourceLine(SourceLoc loc, SourceLoc::IndexTy* lineBeg) const {
  const SourceData* data = getSourceData(loc.getFileID());
  string_view source = data->str;

  auto pair = searchLineTable(data, loc);

  std::size_t beg = pair.first, end = beg;

  if (lineBeg) (*lineBeg) = beg;

  for (; end < source.size(); end++) {
    if (source[end] == '\n' || source[end] == '\r')
      break;
  }

  return source.substr(beg, end-beg);
}

static void skipBom(std::ifstream& in, std::streampos size) {
  // Can retrieve at least 3 bytes
  if(size >= 3) {
    char bom[3];
    // Read the bom
    in.read(bom, 3);
    // Check if it's a bom
    bool has = utf8::starts_with_bom(bom, bom+3);
    // Skip the bom if there's one, or rewind if there isn't one.
    in.seekg(has ? 3 : 0);
  }
}

FileID SourceManager::loadFromFile(const std::string & path) {
  std::ifstream in(path,  std::ios::in | std::ios::ate | std::ios::binary);
  if (in) {
    // Get size of file + rewind
    auto size = in.tellg();
    in.seekg(0);
    // Skip the UTF8 BOM
    skipBom(in, size);
    auto beg = (std::istreambuf_iterator<char>(in));
    auto end = (std::istreambuf_iterator<char>());
    auto pair = sources_.insert(std::pair<FileID,SourceData>(generateNewFileID(),
      SourceData(
        path,
        (std::string(beg, end))
      )
    ));
    return (pair.first)->first;
  }
  return FileID();
}

FileID SourceManager::loadFromString(const std::string& str, const std::string& name) {
  auto pair = sources_.insert({generateNewFileID(),SourceData(name,str)});
  return (pair.first)->first;
}

FileID SourceManager::generateNewFileID() const {
  // The newly generated fileID is always the size of source_ +1, since 0 is the invalid value for FileIDs
  FileID::IDTy id = static_cast<FileID::IDTy>(sources_.size() + 1);
  assert(id != INVALID_FILEID_VALUE);
  return id;
}

void SourceManager::calculateLineTable(const SourceData* data) const {
  std::size_t size = data->str.size();
  CompleteLoc::LineTy line = 1;
  // Mark the index 0 as first line.
  data->lineTable_[0] = 1;
  line++;
  for (std::size_t idx = 0; idx < size; idx++) {
    // Supported line endings : \r\n, \n
    // Just need to add +1 to the index in both cases to mark the beginning
    // of the line as the first character after \n
    if (data->str[idx] == '\n') {
      data->lineTable_[idx+1] = line;
      line++;
    }
  }

  data->calculatedLineTable_ = true;
}

std::pair<SourceLoc::IndexTy, CompleteLoc::LineTy>
SourceManager::searchLineTable(const SourceData* data, const SourceLoc& loc) const {
  if (!data->calculatedLineTable_)
    calculateLineTable(data);
  else {
    // Line table was already calculated, check if the cached search result matches.
    // if it does match, return it.
    if(data->lastLTSearch_.first == loc)
      return data->lastLTSearch_.second;
  }

  auto it = data->lineTable_.lower_bound(loc.getIndex());

  bool exactMatch = false;
  if(it != data->lineTable_.end())
    exactMatch = (it->first == loc.getIndex());

  std::pair<SourceLoc::IndexTy, CompleteLoc::LineTy> rtr;
  if (!exactMatch && (it != data->lineTable_.begin()))
    rtr = *(--it);
  else 
    rtr = *it;
  data->lastLTSearch_ = {loc, rtr};
  return rtr;
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

bool SourceLoc::comesBefore(SourceLoc other) const {
  if(fid_ != other.fid_) return false;
  return idx_ < other.idx_;
}

std::string SourceLoc::toString(const SourceManager& srcMgr) const {
  if (!isValid()) return "";
  auto cloc = srcMgr.getCompleteLoc(*this);
  std::stringstream ss;
  ss << cloc.line << ":" << cloc.column;
  return ss.str();
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

bool SourceRange::contains(SourceLoc loc) const {
  SourceLoc beg = getBegin();
  SourceLoc end = getEnd();
  if(beg.getFileID() != loc.getFileID())
    return false;
  auto begIdx = beg.getIndex();
  auto endIdx = end.getIndex();
  auto locIdx = loc.getIndex();
  return (begIdx >= locIdx) && (locIdx <= endIdx);
}

bool SourceRange::contains(SourceRange range) const {
  return contains(range.getBegin()) && contains(range.getEnd());
}

FileID SourceRange::getFileID() const {
  return sloc_.getFileID();
}

std::string SourceRange::toString(const SourceManager& srcMgr) const {
  std::stringstream ss;
  if (!isValid()) return "";

  auto beg = srcMgr.getCompleteLoc(getBegin());
  ss << beg.line << ":" << beg.column;

  if (offset_ == 0)
    return ss.str();

  auto end = srcMgr.getCompleteLoc(getEnd());

  if (beg.line == end.line)
    ss << "-" << end.column;
  else
    ss << "-" << end.line << ":" << end.column;
  return ss.str();
}
