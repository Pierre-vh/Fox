//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
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
// CompleteLoc
//----------------------------------------------------------------------------//

CompleteLoc::CompleteLoc(string_view fName, line_type ln, col_type col) 
  : fileName(fName), line(ln), column(col) {}

bool CompleteLoc::operator==(const CompleteLoc& other) const {
  return (fileName == other.fileName)
      && (line == other.line) 
      && (column == other.column);
}

bool CompleteLoc::operator!=(const CompleteLoc& other) const {
  return !((*this) == other);
}

std::string CompleteLoc::toString(bool printFilename) const {
  std::stringstream ss;
  if(printFilename)
    ss << fileName << ':';
  ss << line << ":" << column;
  return ss.str();
}

//----------------------------------------------------------------------------//
// CompleteRange
//----------------------------------------------------------------------------//

CompleteRange::CompleteRange(string_view fName, line_type begLine, col_type begCol,
  line_type endLine, col_type endCol) : fileName(fName), begLine(begLine),
  begColumn(begCol), endLine(endLine), endColumn(endCol) {}

bool CompleteRange::operator==(const CompleteRange& other) const {
  return (fileName == other.fileName)
    && (begLine == other.begLine)
    && (begColumn == other.begColumn)
    && (endLine == other.endLine)
    && (endColumn == other.endColumn);
}
bool CompleteRange::operator!=(const CompleteRange& other) const {
  return !((*this) == other);
}

std::string CompleteRange::toString(bool printFilename) const {
  std::stringstream ss;
  if (printFilename)
    ss  << fileName << ':';
  ss << begLine << ':' << begColumn << '-' << endLine << ':' << endColumn;
  return ss.str();
}

//----------------------------------------------------------------------------//
// FileID
//----------------------------------------------------------------------------//

FileID::FileID(std::size_t value) {
  assert(value < npos && "Index too big for FileID!");
  idx_ = static_cast<IDTy>(value);
}

bool FileID::isValid() const {
  return idx_ != npos;
}

FileID::operator bool() const {
  return isValid();
}

bool FileID::operator==(const FileID other) const {
  return idx_ == other.idx_;
}

bool FileID::operator!=(const FileID other) const {
  return !(*this == other);
}

bool FileID::operator <(const FileID other) const {
  return (idx_ < other.idx_);
}

std::ostream& fox::operator<<(std::ostream& os, FileID file) {
  if(file.isValid())
    os << "FileID(" << file.idx_ << ")";
  else 
    os << "FileID(invalid)";
  return os;
}


//----------------------------------------------------------------------------//
// SourceLoc
//----------------------------------------------------------------------------//

SourceLoc::SourceLoc() : fid_(FileID()), idx_(0) {}

SourceLoc::SourceLoc(FileID fid, IndexTy idx) : fid_(fid), idx_(idx) {}

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
  assert((fid_ == other.fid_) && "SourceLocs cannot be compared!");
  return idx_ < other.idx_;
}

std::ostream& fox::operator<<(std::ostream& os, SourceLoc loc) {
  if(loc.isValid())
    os << "SourceLoc(" << loc.getFileID() << ", " << loc.idx_ << ")";
  else 
    os << "SourceLoc(invalid)";
  return os;
}

//----------------------------------------------------------------------------//
// SourceRange
//----------------------------------------------------------------------------//

SourceRange::SourceRange(SourceLoc sloc, OffsetTy offset):
  loc_(sloc), offset_(offset) {}

SourceRange::SourceRange(SourceLoc a, SourceLoc b) {
  // a and b must belong to the same file in all cases!
  assert(a.getFileID() == b.getFileID() && "A and B are from different files");
  if (a.getRawIndex() < b.getRawIndex()) {
    // a is the first sloc
    loc_ = a;
    offset_ = static_cast<OffsetTy>(b.getRawIndex() - a.getRawIndex());
  }
  else if (a.getRawIndex() > b.getRawIndex()) {
    // b is the first sloc
    loc_ = b;
    offset_ = static_cast<OffsetTy>(a.getRawIndex() - b.getRawIndex());
  }
  else  {
    // a == b
    loc_ = a;
    offset_ = 0;
  }
}

SourceRange::SourceRange(): loc_(SourceLoc()), offset_(0) {}

bool SourceRange::isValid() const {
  return (bool)loc_;
}

SourceRange::operator bool() const {
  return isValid();
}

SourceLoc SourceRange::getBeginLoc() const {
  return loc_;
}

SourceRange::OffsetTy SourceRange::getRawOffset() const {
  return offset_;
}

SourceLoc SourceRange::getEndLoc() const {
  return SourceLoc(loc_.fid_, loc_.idx_ + offset_);
}

bool SourceRange::contains(SourceLoc loc) const {
  SourceLoc beg = getBeginLoc();
  SourceLoc end = getEndLoc();
  assert((beg.getFileID() == loc.getFileID()) && 
    "'loc' is not in the same file as this SourceRange");
  auto begIdx = beg.getRawIndex();
  auto endIdx = end.getRawIndex();
  auto locIdx = loc.getRawIndex();
  return (begIdx <= locIdx) && (locIdx <= endIdx);
}

bool SourceRange::contains(SourceRange range) const {
  return contains(range.getBeginLoc()) && contains(range.getEndLoc());
}

FileID SourceRange::getFileID() const {
  return loc_.getFileID();
}

std::ostream& fox::operator<<(std::ostream& os, SourceRange range) {
  if(range.isValid())
    os << "SourceRange(" << range.getBeginLoc() << ", " << range.getEndLoc() << ")";
  else 
    os << "SourceRange(invalid)";
  return os;
}

//----------------------------------------------------------------------------//
// SourceManager
//----------------------------------------------------------------------------//

string_view SourceManager::getFileContent(FileID fid) const {
  auto data = getData(fid);
  return data->content;
}

string_view SourceManager::getFileName(FileID fid) const {
  auto data = getData(fid);
  return data->name;
}

std::pair<SourceManager::line_type, SourceManager::col_type>
SourceManager::calculateLineAndColumn(const Data* data, 
                                      SourceLoc::IndexTy idx) const {
  assert(data && "null data!");
  assert((idx <= data->content.size()) && "out-of-range index!");
  // if the SourceLoc points to a fictive location just past the end
  // of the source, remove the extra column to avoid out_of_range errors
  bool isPastTheEnd = (idx == data->content.size());
  if (isPastTheEnd)
    idx--;

  line_type line = 0;
  col_type col = 0;

  auto entry = searchLineTable(data, idx);
  bool exactMatch = (entry.first == idx);
  if (exactMatch) {
    line = entry.second;
    col = 1;
  }
  else {
    line = entry.second;
    auto str_beg = data->content.c_str(); // Pointer to the first 
                                          // character of the string
    auto raw_col = utf8::distance(str_beg + entry.first, str_beg + idx);
    col = static_cast<col_type>(raw_col + 1);
  }

  // Return, adding back the extra column if needed
  return {line, (isPastTheEnd ? ++col : col)};
}

const SourceManager::Data*
SourceManager::getData(FileID file) const {
  auto idx = file.idx_;
  assert(file.isValid() && "FileID is not valid");
  assert(idx < datas_.size() && "out-of-range FileID");
  return datas_[idx].get();
}

SourceManager::line_type SourceManager::getLineNumber(SourceLoc loc) const {
  auto result = searchLineTable(
    getData(loc.getFileID()), loc.getRawIndex());
  return result.second;
}

CompleteLoc SourceManager::getCompleteLoc(SourceLoc sloc) const {
  const Data* data = getData(sloc.getFileID());
  auto lineAndCol = calculateLineAndColumn(data, sloc.getRawIndex());
  return CompleteLoc(
    data->name,
    lineAndCol.first,
    lineAndCol.second
  );
}

CompleteRange SourceManager::getCompleteRange(SourceRange range) const {
  const Data* data = getData(range.getFileID());
  auto begIdx = range.getBeginLoc().getRawIndex();
  auto endIdx = range.getEndLoc().getRawIndex();
  
  line_type begLine = 0, endLine = 0;
  col_type begCol = 0, endCol = 0;

  std::tie(begLine, begCol) = calculateLineAndColumn(data, begIdx);
 
  if (range.getRawOffset() == 0) {
    endLine = begLine;
    endCol = begCol;
  }
  else
    std::tie(endLine, endCol) = calculateLineAndColumn(data, endIdx);
  
  return CompleteRange(
    data->name,
    begLine,
    begCol,
    endLine,
    endCol
  );
}

string_view 
SourceManager::getLineAt(SourceLoc loc, SourceLoc* lineBeg) const {
  // Retrieve the data
  const Data* data = getData(loc.getFileID());
  // Check that our index is valid
  assert(isIndexValid(data, loc.getRawIndex()));
  // Retrieve the source
  string_view source = data->content;
  // Search the line table
  auto pair = searchLineTable(data, loc.getRawIndex());
  std::size_t beg = pair.first, end = beg;
  // Give the loc of the beginning of the line to the caller if it wants it.
  if (lineBeg) (*lineBeg) = SourceLoc(loc.getFileID(), beg);
  // Find the end of the line
  for (; end < source.size(); end++) {
    if (source[end] == '\n' || source[end] == '\r')
      break;
  }
  // Create the string
  return source.substr(beg, end-beg);
}

SourceLoc 
SourceManager::advance(SourceLoc loc, std::size_t count) const {
  if(count == 0) return loc;

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

  if (cur != end) {
    // If this isn't a past-the-end SourceLoc
    for (std::size_t k = 0; k < count; k++) {
      if(next == end) break;
      utf8::next(next, end);
    }
    // Calculate the offset
    std::size_t offset = std::distance(cur, next);
    // Recompose the SourceLoc
    return SourceLoc(file, raw+offset);
  }

  // If this is a past-the-end SourceLoc, just return it since it cannot be
  // incremented.
  return loc;
}

std::size_t 
SourceManager::getLengthInCodepoints(SourceRange range) const {
  SourceLoc beg = range.getBeginLoc();
  SourceLoc end = range.getEndLoc();
  // If the SourceLocs are identical, just return 1.
  if(beg == end) return 1;

  // Retrieve the source file
  const Data* data = getData(range.getFileID());

  // Check that our locs are valid
  assert(isIndexValid(data, beg.getRawIndex()) && "a is not valid");
  assert(isIndexValid(data, end.getRawIndex()) && "b is not valid");

  // Calculate the needed iterators
  string_view source = data->content;
  auto begIt = source.begin()+beg.getRawIndex();
  auto endIt = source.begin()+end.getRawIndex();

  // Calculate the distance 
  std::size_t distance = utf8::distance(begIt, endIt);
  assert(distance && "distance is zero!");
  // Return distance+1, because distance doesn't count the first
  // character.
  return distance+1;
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

    if((maybeBOM[0] == '\xFF') && (maybeBOM[1] == '\xFE'))
      hasUTF16Bom = true;
    else if((maybeBOM[0] == '\xFE') && (maybeBOM[1] == '\xFF'))
      hasUTF16Bom = true;

    in.seekg(0);
    if(hasUTF16Bom) return false;
  }

  // The encoding should be ok
  return true;
}

std::pair<FileID, SourceManager::ReadFileResult>
SourceManager::readFile(string_view path) {
  std::ifstream in(path.to_string(),  
    std::ios::in | std::ios::ate | std::ios::binary);
  if(!in)
    return {FileID(), ReadFileResult::NotFound};

  // Get size of file + rewind
  auto size = in.tellg();
  in.seekg(0);

  // Skip the UTF8 BOM if there's one
  if(!checkEncoding(in, size))
    return {FileID(), ReadFileResult::InvalidEncoding};

  // Create the iterators
  auto beg = (std::istreambuf_iterator<char>(in));
  auto end = (std::istreambuf_iterator<char>());
  // Insert the data
  FileID file = insertData(std::make_unique<Data>(path, beg, end));
  return {file, ReadFileResult::Ok};
}

FileID 
SourceManager::loadFromString(string_view str, string_view name) {
  return insertData(std::make_unique<Data>(name, str));
}

void SourceManager::calculateLineTable(const Data* data) const {
  std::size_t size = data->content.size();
  CompleteLoc::line_type line = 1;
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

std::pair<SourceLoc::IndexTy, SourceManager::line_type>
SourceManager::searchLineTable(const Data* data, SourceLoc::IndexTy idx) const {
  if (!data->calculatedLineTable_)
    calculateLineTable(data);
  else {
    // Line table was already calculated, check if the cached search result matches.
    // if it does match, return it.
    if(data->lastLTSearch_.first == idx)
      return data->lastLTSearch_.second;
  }

  auto it = data->lineTable_.lower_bound(idx);

  bool exactMatch = false;
  if(it != data->lineTable_.end())
    exactMatch = (it->first == idx);

  std::pair<SourceLoc::IndexTy, CompleteLoc::line_type> rtr;
  if (!exactMatch && (it != data->lineTable_.begin()))
    rtr = *(--it);
  else 
    rtr = *it;
  data->lastLTSearch_ = {idx, rtr};
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

std::string fox::toString(SourceManager::ReadFileResult status) {
  using FS = SourceManager::ReadFileResult;
  switch(status) {
    case FS::Ok:
      return "Ok";
    case FS::NotFound:
      return "File Not Found";
    case FS::InvalidEncoding:
      return "File Encoding Not Supported";
    default:
      fox_unreachable("unknown ReadFileResult");
  }
}