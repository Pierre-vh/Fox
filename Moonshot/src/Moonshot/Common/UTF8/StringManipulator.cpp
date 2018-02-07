////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : StringManipulator.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "StringManipulator.h"

using namespace Moonshot;

void UTF8::skipBOM(std::string::iterator & it, std::string::iterator end)
{
	if (utf8::starts_with_bom(it, end))
		utf8::next(it, end);
}

void UTF8::append(std::string & str, const CharType & ch)
{
	utf8::append(ch, std::back_inserter(str));
}


UTF8::StringManipulator::StringManipulator()
{
}


UTF8::StringManipulator::~StringManipulator()
{

}

std::string UTF8::StringManipulator::getStr() const
{
	return str_;
}

void UTF8::StringManipulator::setStr(const std::string & str)
{
	str_ = str;
	reset();
}

std::string UTF8::StringManipulator::wcharToStr(const wchar_t & wc) const
{
	std::string rtr;
	append(rtr, wc);
	return rtr;
}

void UTF8::StringManipulator::reset()
{
	// set iterators
	iter_ = str_.begin();
	end_ = str_.end();
	beg_ = str_.begin();
	// skip  bom if there is one
	UTF8::skipBOM(iter_,end_);
}

void UTF8::StringManipulator::advance(const std::size_t & ind)
{
	utf8::advance(iter_, ind, str_.end());
}

CharType UTF8::StringManipulator::currentChar() const
{
	if (iter_ == str_.end())
		return L'\0';
	return utf8::peek_next(iter_,end_);
}

CharType UTF8::StringManipulator::getChar(std::size_t ind) const
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

std::string UTF8::StringManipulator::substring(std::size_t beg, const std::size_t & leng) const
{
	std::string cpy = str_;

	std::string::iterator it = cpy.begin();
	UTF8::skipBOM(it, cpy.end());
	
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

CharType UTF8::StringManipulator::peekFirst() const
{
	if (getSize()) // string needs at least 1 char
		return utf8::peek_next(beg_,end_);
	return L'\0';
}

CharType UTF8::StringManipulator::peekNext() const
{
	if (isAtEndOfStr())
		return L'\0';

	auto tmpit = iter_;
	utf8::advance(tmpit, 1,end_); // peek_next in utfcpp returns what we expect to be the "next" character, so we need to advance
	if(tmpit != str_.end())
		return utf8::peek_next(tmpit,end_);
	return L'\0';
}

CharType UTF8::StringManipulator::peekPrevious() const
{
	if (iter_ == str_.begin())
		return L'\0';

	auto tmpiter = iter_;
	return utf8::previous(tmpiter,beg_);
}

CharType UTF8::StringManipulator::peekBack() const
{
	auto tmp = end_;
	if (getSize()) // string needs at least 1 char
		return utf8::prior(tmp,beg_);
	return L'\0';
}

std::size_t UTF8::StringManipulator::getSize() const
{
	return utf8::distance(str_.begin(), str_.end());
}

bool UTF8::StringManipulator::isAtEndOfStr() const
{
	return iter_ == str_.end();
}