////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : StringManipulator.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "StringManipulator.hpp"
#include "utfcpp/utf8.hpp"

using namespace Moonshot;

UTF8::StringManipulator::StringManipulator(const std::string & str)
{
	setStr(str);
}

std::string UTF8::StringManipulator::getStrCpy() const
{
	return str();
}

const std::string * UTF8::StringManipulator::getStrPtr() const
{
	if (std::holds_alternative<std::string>(data_))
		return &(std::get<std::string>(data_));
	else if (std::holds_alternative<std::string*>(data_))
		return std::get<std::string*>(data_);
	else
		throw std::exception("unknown variant.");
}

void UTF8::StringManipulator::setStr(std::string * str)
{
	data_ = str;
	reset();
}

void UTF8::StringManipulator::setStr(const std::string & str)
{
	data_ = str;
	reset();
}

bool UTF8::StringManipulator::isUsingAPointer() const
{
	return std::holds_alternative<std::string*>(data_);
}
bool UTF8::StringManipulator::isUsingACopy() const
{
	return std::holds_alternative<std::string>(data_);
}

std::string UTF8::StringManipulator::wcharToStr(const CharType & wc)
{
	std::string rtr;
	append(rtr, wc);
	return rtr;
}

void UTF8::StringManipulator::removeBOM(std::string & str)
{
	if (utf8::starts_with_bom(str.begin(), str.end()))
	{
		std::string::iterator it = str.begin();
		utf8::next(it,str.end());
		str.erase(str.begin(), it);
	}
}

void UTF8::StringManipulator::skipBOM(std::string::iterator & it, std::string::iterator end)
{
	if (utf8::starts_with_bom(it, end))
		utf8::next(it, end);
}

void UTF8::StringManipulator::append(std::string & str, const CharType & ch)
{
	utf8::append(ch, std::back_inserter(str));
}

std::size_t UTF8::StringManipulator::indexOfCurrentCharacter() const
{
	return utf8::distance(beg_, iter_);
}

std::size_t UTF8::StringManipulator::rawIndexOfCurrentCharacter() const
{
	return std::distance(beg_, iter_);
}

std::pair<uint32_t, uint16_t> UTF8::StringManipulator::getLineAndColumnForIndex(const std::string & locstr, const std::size_t & idx)
{
	uint32_t line	= 1;
	uint16_t col	= 0;
	// Count the number of newlines
	line += std::count(locstr.begin(), locstr.begin() + idx, '\n');

	// Go backwards from idx and count the number of character until \n or beg of file to get the column
	for (auto it = (locstr.begin() + idx); (*it != '\n') && (it != locstr.begin()); utf8::prior(it,locstr.begin()))
	{
		if (*it == '\t')
			col += 4;
		else
			col++;
	}
	return { line,col };
}

std::pair<uint32_t, uint16_t> UTF8::StringManipulator::getLineAndColumnForCurrentCharacter() const
{
	return getLineAndColumnForIndex(str(), rawIndexOfCurrentCharacter());
}

void UTF8::StringManipulator::reset()
{
	// set iterators
	iter_ = str().begin();
	end_ = str().end();
	beg_ = str().begin();
	// skip  bom if there is one
	UTF8::StringManipulator::skipBOM(iter_,end_);
}

void UTF8::StringManipulator::advance(const std::size_t & ind)
{
	if (iter_ != end_)
		utf8::advance(iter_, ind, end_);
}

void UTF8::StringManipulator::goBack(const std::size_t & ind)
{
	for (std::size_t k = 0; k < ind; k++)
		utf8::prior(iter_, beg_);
}

CharType UTF8::StringManipulator::getCurrentChar() const
{
	if (iter_ == end_)
		return L'\0';
	return utf8::peek_next(iter_,end_);
}

CharType UTF8::StringManipulator::getChar(std::size_t ind) const
{
	std::string::iterator tmpit = beg_;

	utf8::advance(tmpit,ind, end_);

	if (tmpit != end_)
		return utf8::peek_next(tmpit, end_);
	return L'\0';
}

std::string UTF8::StringManipulator::substring(std::size_t beg, const std::size_t & leng) const
{
	std::string::iterator tmpit = beg_;
	
	utf8::advance(tmpit, beg, end_);

	std::string rtr;

	for (std::size_t ind(0); ind < leng; ind++)
	{
		const auto ch = utf8::next(tmpit,end_);
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
	if(tmpit != str().end())
		return utf8::peek_next(tmpit,end_);
	return L'\0';
}

CharType UTF8::StringManipulator::peekPrevious() const
{
	if (iter_ == beg_)
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
	return utf8::distance(beg_,end_);
}

bool UTF8::StringManipulator::isAtEndOfStr() const
{
	return iter_ == end_;
}

std::string & UTF8::StringManipulator::str()
{
	if (std::holds_alternative<std::string>(data_))
		return std::get<std::string>(data_);
	else if (std::holds_alternative<std::string*>(data_))
		return *(std::get<std::string*>(data_));
	else
		throw std::exception("unknown variant.");
}

const std::string & UTF8::StringManipulator::str() const
{
	if (std::holds_alternative<std::string>(data_))
		return std::get<std::string>(data_);
	else if (std::holds_alternative<std::string*>(data_))
		return *(std::get<std::string*>(data_));
	else
		throw std::exception("unknown variant.");
}