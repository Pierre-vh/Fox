//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
// See LICENSE.txt for license info.            
// File : Source.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Common/SourceLoc.hpp"
#include "Fox/Common/SourceManager.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/Common/StringManipulator.hpp"
#include "utfcpp/utf8.hpp"
#include <fstream>
#include <sstream>

using namespace fox;

//----------------------------------------------------------------------------//
// FileID
//----------------------------------------------------------------------------//

FileID::FileID(std::size_t value) {
  assert(value < npos && "Index too big for FileID!");
  value_ = static_cast<IDTy>(value);
}

bool FileID::isValid() const {
  return value_ != npos;
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

//----------------------------------------------------------------------------//
// SourceLoc
//----------------------------------------------------------------------------//

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

SourceLoc::IndexTy SourceLoc::getRawIndex() const {
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

//----------------------------------------------------------------------------//
// SourceRange
//----------------------------------------------------------------------------//

SourceRange::SourceRange(SourceLoc sloc, OffsetTy offset):
  sloc_(sloc), offset_(offset) {}

SourceRange::SourceRange(SourceLoc a, SourceLoc b) {
  // a and b must belong to the same file in all cases!
  assert(a.getFileID() == b.getFileID() && "A and B are from different files");
  if (a.getRawIndex() < b.getRawIndex()) {
    // a is the first sloc
    sloc_ = a;
    offset_ = static_cast<OffsetTy>(b.getRawIndex() - a.getRawIndex());
  }
  else if (a.getRawIndex() > b.getRawIndex()) {
    // b is the first sloc
    sloc_ = b;
    offset_ = static_cast<OffsetTy>(a.getRawIndex() - b.getRawIndex());
  }
  else  {
    // a == b
    sloc_ = a;
    offset_ = 0;
  }
}

SourceRange::SourceRange(): sloc_(SourceLoc()), offset_(0) {}

bool SourceRange::isValid() const {
  return (bool)sloc_;
}

SourceRange::operator bool() const {
  return isValid();
}

SourceLoc SourceRange::getBegin() const {
  return sloc_;
}

SourceRange::OffsetTy SourceRange::getRawOffset() const {
  return offset_;
}

SourceLoc SourceRange::getEnd() const {
  return SourceLoc(sloc_.getFileID(), sloc_.getRawIndex() + offset_);
}

bool SourceRange::isOnlyOneCharacter() const {
  return (offset_ == 0);
}

bool SourceRange::contains(SourceLoc loc) const {
  SourceLoc beg = getBegin();
  SourceLoc end = getEnd();
  if(beg.getFileID() != loc.getFileID())
    return false;
  auto begIdx = beg.getRawIndex();
  auto endIdx = end.getRawIndex();
  auto locIdx = loc.getRawIndex();
  assert(true);
  return (begIdx <= locIdx) && (locIdx <= endIdx);
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

//----------------------------------------------------------------------------//
// SourceManager
//----------------------------------------------------------------------------//

string_view SourceManager::getFileName(FileID fid) const {
  auto data = getData(fid);
  return data->content;
}

string_view SourceManager::getFileContent(FileID fid) const {
  auto data = getData(fid);
  return data->name;
}

const SourceManager::Data*
SourceManager::getData(FileID file) const {
  assert(file.isValid() && "FileID is not valid");
  assert(file.getRaw() < datas_.size() && "out-of-range FileID");
  return datas_[file.getRaw()].get();
}

CompleteLoc::LineTy SourceManager::getLineNumber(SourceLoc loc) const {
  auto result = searchLineTable(
    getData(loc.getFileID()), loc);
  return result.second;
}

SourceRange SourceManager::getRangeOfFile(FileID file) const {
  using OffTy = SourceRange::OffsetTy;
  
  // Begin SourceLoc is always (file, 0)
  SourceLoc begin(file, 0);

  // Calculate end
  const Data* data = getData(file);
  std::size_t size = data->content.size();

  // Check that the size isn't too big, just to be sure.
  assert(size < std::numeric_limits<OffTy>::max() &&
    "Can't create a file-wide SourceRange for this file: file is too large!");
  
  // Return the SourceRange
  return SourceRange(begin, static_cast<OffTy>(size));
}

CompleteLoc SourceManager::getCompleteLoc(SourceLoc sloc) const {
  const Data* fdata = getData(sloc.getFileID());

  auto idx = sloc.getRawIndex();
  assert((idx <= fdata->content.size()) && "SourceLoc is Out-of-Range");

  // if the SourceLoc points to a fictive location just past the end
  // of the source, remove the extra column to avoid out_of_range errors
  bool isOutOfRange = (idx == fdata->content.size());
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
    auto str_beg = fdata->content.c_str(); // Pointer to the first 
                                       // character of the string
    auto raw_col = utf8::distance(str_beg + entry.first, str_beg + idx);
    col = static_cast<CompleteLoc::ColTy>(raw_col+1);
  }

  // Add back the extra column if needed
  if (isOutOfRange)
    col++;

  return CompleteLoc(
    fdata->name,
    line,
    col
  );
}

string_view 
SourceManager::getSourceLine(SourceLoc loc, SourceLoc::IndexTy* lineBeg) const {
  // Retrieve the data
  const Data* data = getData(loc.getFileID());
  // Check that our index is valid
  assert(isIndexValid(data, loc.getRawIndex()));
  // Retrieve the source
  string_view source = data->content;
  // Search the line table
  auto pair = searchLineTable(data, loc);
  std::size_t beg = pair.first, end = beg;
  // Give the index of the beginning of the line to the caller if it wants it.
  if (lineBeg) (*lineBeg) = beg;
  // Find the end of the line
  for (; end < source.size(); end++) {
    if (source[end] == '\n' || source[end] == '\r')
      break;
  }
  // Create the string
  return source.substr(beg, end-beg);
}

SourceLoc SourceManager::getNextCodepointSourceLoc(SourceLoc loc) const {
  // First, retrieve the Data.
  FileID file = loc.getFileID();
  const Data* data = getData(file);
  // Check that our loc is valid
  auto raw = loc.getRawIndex();
  assert(isIndexValid(data, raw));
  
  // Prepare the iterators
  auto cur = data->content.begin()+raw;
  auto next = cur;
  auto end = data->content.end();

  // If this isn't a past-the-end SourceLoc
  if(cur != end) {
    // Get the next character in the sequence
    utf8::next(next, end);
    // Calculate the offset
    std::size_t offset = std::distance(cur, next);
    // Recompose the SourceLoc
    return SourceLoc(file, raw+offset);
  }
  // If this is a past-the-end SourceLoc, just it since it cannot be
  // incremented.
  return loc;
}

// Checks the encoding of the file, skipping the UTF-8 bom if it's present.
// Returns false if the encoding of the file is not supported.
static bool checkEncoding(std::ifstream& in, std::streampos size) {
  // Can retrieve at least 3 bytes
  if(size >= 3) {
    char maybeBOM[3];
    // Read the bom
    in.read(maybeBOM, 3);
    // Check if it's a UTF8 bom
    bool hasUTF8Bom = utf8::starts_with_bom(maybeBOM, maybeBOM+3);
    // Skip the bom if there's one, or rewind if there isn't one.
    in.seekg(hasUTF8Bom ? 3 : 0);
    // We can be sure it's UTF8 if it has a bom
    if(hasUTF8Bom)
      return true;
  }


  // Now, check for invalid encodings:
  //  UTF-16 BOM: 0xFEFF or 0xFFFE
  if(size >= 2) {
    char maybeBOM[2];
    in.read(maybeBOM, 2);

    bool hasUTF16Bom = false;

    if((maybeBOM[0] == 0xFF) && (maybeBOM[1] == 0xFE))
      hasUTF16Bom = true;
    else if((maybeBOM[0] == 0xFE) && (maybeBOM[1] == 0xFF))
      hasUTF16Bom = true;

    in.seekg(0);
    if(hasUTF16Bom) return false;
  }

  // The encoding should be ok
  return true;
}

std::pair<FileID, SourceManager::FileStatus>
SourceManager::readFile(string_view path) {
  std::ifstream in(path.to_string(),  
    std::ios::in | std::ios::ate | std::ios::binary);
  if(!in)
    return {FileID(), FileStatus::NotFound};

  // Get size of file + rewind
  auto size = in.tellg();
  in.seekg(0);

  // Skip the UTF8 BOM if there's one
  if(!checkEncoding(in, size))
    return {FileID(), FileStatus::InvalidEncoding};

  // Create the iterators
  auto beg = (std::istreambuf_iterator<char>(in));
  auto end = (std::istreambuf_iterator<char>());
  // Insert the data
  FileID file = insertData(std::make_unique<Data>(path, beg, end));
  return {file, FileStatus::Ok};
}

FileID 
SourceManager::loadFromString(string_view str, string_view name) {
  return insertData(std::make_unique<Data>(name, str));
}

void SourceManager::calculateLineTable(const Data* data) const {
  std::size_t size = data->content.size();
  CompleteLoc::LineTy line = 1;
  // Mark the index 0 as first line.
  data->lineTable_[0] = 1;
  line++;
  for (std::size_t idx = 0; idx < size; idx++) {
    // Supported line endings : \r\n, \n
    // Just need to add +1 to the index in both cases to mark the beginning
    // of the line as the first character after \n
    if (data->content[idx] == '\n') {
      data->lineTable_[idx+1] = line;
      line++;
    }
  }

  data->calculatedLineTable_ = true;
}

bool 
SourceManager::isIndexValid(const Data* data, SourceLoc::IndexTy idx) const {
  // The index is valid if it's smaller or equal to .size(). It can be
  // equal to size in the case of a "past the end" loc.
  return idx <= data->content.size();
}

std::pair<SourceLoc::IndexTy, CompleteLoc::LineTy>
SourceManager::searchLineTable(const Data* data, const SourceLoc& loc) const {
  if (!data->calculatedLineTable_)
    calculateLineTable(data);
  else {
    // Line table was already calculated, check if the cached search result matches.
    // if it does match, return it.
    if(data->lastLTSearch_.first == loc)
      return data->lastLTSearch_.second;
  }

  auto it = data->lineTable_.lower_bound(loc.getRawIndex());

  bool exactMatch = false;
  if(it != data->lineTable_.end())
    exactMatch = (it->first == loc.getRawIndex());

  std::pair<SourceLoc::IndexTy, CompleteLoc::LineTy> rtr;
  if (!exactMatch && (it != data->lineTable_.begin()))
    rtr = *(--it);
  else 
    rtr = *it;
  data->lastLTSearch_ = {loc, rtr};
  return rtr;
}

FileID SourceManager::insertData(std::unique_ptr<Data> data) {
  // Construct a unique_ptr in place with the freshly created Data*.
  datas_.emplace_back(std::move(data));
  // FileID is the index of the last element, so .size()-1
  return FileID(datas_.size()-1);
}

//----------------------------------------------------------------------------//
// Others
//----------------------------------------------------------------------------//

std::string fox::toString(SourceManager::FileStatus status) {
  using FS = SourceManager::FileStatus;
  switch(status) {
    case FS::Ok:
      return "Ok";
    case FS::NotFound:
      return "File Not Found";
    case FS::InvalidEncoding:
      return "File Encoding Not Supported";
    default:
      fox_unreachable("unknown FileStatus");
  }
}