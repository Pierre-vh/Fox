//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : StringManipulator.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Common/StringManipulator.hpp"
#include "utfcpp/utf8.hpp"
#include <cassert>

using namespace fox;

StringManipulator::StringManipulator(nonstd::string_view str)
{
  setStr(str);
}

nonstd::string_view StringManipulator::getStr() const
{
  return str_;
}

void StringManipulator::setStr(nonstd::string_view str)
{
  str_ = str;
  reset();
}

std::string StringManipulator::charToStr(FoxChar wc)
{
  std::string rtr;
  append(rtr, wc);
  return rtr;
}

void StringManipulator::removeBOM(std::string & str)
{
  if (utf8::starts_with_bom(str.begin(), str.end()))
  {
    std::string::iterator it = str.begin();
    utf8::next(it,str.end());
    str.erase(str.begin(), it);
  }
}

void StringManipulator::append(std::string & str, FoxChar ch)
{
  utf8::append(ch, std::back_inserter(str));
}

std::size_t StringManipulator::getIndexInCodepoints() const
{
  return utf8::distance(beg_, iter_);
}

std::size_t StringManipulator::getIndexInBytes() const
{
  return std::distance(beg_, iter_);
}

void StringManipulator::reset()
{
  // set iterators
  iter_ = str_.begin();
  end_ = str_.end();
  beg_ = str_.begin();
  // skip  bom if there is one
  StringManipulator::skipBOM(iter_,end_);
}

void StringManipulator::advance(const std::size_t & ind)
{
  if (iter_ != end_)
    utf8::advance(iter_, ind, end_);
}

void StringManipulator::goBack(const std::size_t& ind)
{
  for (std::size_t k = 0; k < ind; k++)
    utf8::prior(iter_, beg_);
}

FoxChar StringManipulator::getCurrentChar() const
{
  if (iter_ == end_)
    return L'\0';
  return utf8::peek_next(iter_,end_);
}

FoxChar StringManipulator::getChar(std::size_t ind) const
{
  auto tmpit = beg_;

  utf8::advance(tmpit,ind, end_);

  if (tmpit != end_)
    return utf8::peek_next(tmpit, end_);
  return 0;
}

std::string StringManipulator::substring(std::size_t beg, std::size_t leng) const
{
  auto tmpit = beg_;
  
  utf8::advance(tmpit, beg, end_);

  std::string rtr;

  for (std::size_t ind(0); ind < leng; ind++)
  {
    const auto ch = utf8::next(tmpit,end_);
    append(  rtr, 
        ch
      );
  }
  return rtr;
}

FoxChar StringManipulator::peekFirst() const
{
  if (getSizeInCodepoints()) // string needs at least 1 char
    return utf8::peek_next(beg_,end_);
  return L'\0';
}

FoxChar StringManipulator::peekNext() const
{
  if (eof())
    return L'\0';

  auto tmpit = iter_;
  utf8::advance(tmpit, 1,end_); // peek_next in utfcpp returns what we expect to be the "next" character, so we need to advance
  if(tmpit != str_.end())
    return utf8::peek_next(tmpit,end_);
  return L'\0';
}

FoxChar StringManipulator::peekPrevious() const
{
  if (iter_ == beg_)
    return L'\0';

  auto tmpiter = iter_;
  return utf8::previous(tmpiter,beg_);
}

FoxChar StringManipulator::peekBack() const
{
  auto tmp = end_;
  if (getSizeInCodepoints()) // string needs at least 1 char
    return utf8::prior(tmp,beg_);
  return L'\0';
}

std::size_t StringManipulator::getSizeInCodepoints() const
{
  return utf8::distance(beg_,end_);
}

std::size_t StringManipulator::getSizeInBytes() const
{
  return str_.size();
}

bool StringManipulator::eof() const
{
  return iter_ == end_;
}