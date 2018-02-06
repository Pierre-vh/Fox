////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : UTF8StringManipulator.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "UTF8StringManipulator.h"

using namespace Moonshot;

UTF8StringManipulator::UTF8StringManipulator()
{
}


UTF8StringManipulator::~UTF8StringManipulator()
{
}

std::string UTF8StringManipulator::getStr() const
{
	return str_;
}

void UTF8StringManipulator::setStr(const std::string & str)
{
	str_ = str;
	reset();
}

std::string Moonshot::UTF8StringManipulator::wcharToStr(const wchar_t & wc) const
{
	std::string rtr;
	append(rtr, wc);
	return rtr;
}

void UTF8StringManipulator::append(std::string & str, const CharType & ch) const
{
	utf8::append(ch, std::back_inserter(str));
}

void UTF8StringManipulator::reset()
{
	iter_ = str_.begin();
	if (utf8::is_bom(iter_))
		utf8::advance(iter_, 1, str_.end());

	end_ = str_.end();
	beg_ = str_.begin();
}

void UTF8StringManipulator::advance(const std::size_t & ind)
{
	utf8::advance(iter_, ind, str_.end());
}

CharType UTF8StringManipulator::currentChar()
{
	if (iter_ == str_.end())
		return L'\0';
	return utf8::peek_next(iter_, str_.end());
}

CharType UTF8StringManipulator::getChar(std::size_t ind) const
{
	std::string tmp = str_; // need a temp copy to get non const iterators!

	if (utf8::is_bom(tmp.begin())) // add +1 to skip bom if it has one
		ind += 1;

	std::size_t count = 0, size = getSize();
	std::string::iterator it = tmp.begin(),
		end = tmp.end();
	utf8::advance(it, ind, tmp.end());
	if (it != tmp.end())
		return utf8::peek_next(it, tmp.end());
	return L'\0';
}

std::string UTF8StringManipulator::substring(std::size_t beg, const std::size_t & leng) const
{
	std::string cpy = str_;

	if (utf8::is_bom(cpy.begin())) beg += 1; // If str has a bom, skip it (set beg+=1 so utfcpp ignores the first codepoint)

	std::string::iterator it = cpy.begin();
	utf8::advance(it, beg, cpy.end());

	std::string rtr;

	for (std::size_t ind(0); ind < leng; ind++)
	{
		auto ch = utf8::next(it, cpy.end());
		append(	rtr, 
				ch
			);
	}
	return rtr;
}

CharType UTF8StringManipulator::peekFirst() const
{
	if (getSize()) // string needs at least 1 char
		return utf8::peek_next(beg_,end_);
	return L'\0';
}

CharType UTF8StringManipulator::peekNext() const
{
	if (isAtEndOfStr())
		return L'\0';

	auto tmpit = iter_;
	utf8::advance(tmpit, 1,end_); // peek_next in utfcpp returns what we expect to be the "next" character, so we need to advance
	if(tmpit != str_.end())
		return utf8::peek_next(tmpit,end_);
	return L'\0';
}

CharType UTF8StringManipulator::peekPrevious() const
{
	if (iter_ == str_.begin())
		return L'\0';

	auto tmpiter = iter_;
	return utf8::previous(tmpiter,beg_);
}

CharType UTF8StringManipulator::peekBack() const
{
	auto tmp = end_;
	if (getSize()) // string needs at least 1 char
		return utf8::prior(tmp,beg_);
	return L'\0';
}

std::size_t UTF8StringManipulator::getSize() const
{
	return utf8::distance(str_.begin(), str_.end());
}

bool UTF8StringManipulator::isAtEndOfStr() const
{
	return iter_ == str_.end();
}


