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

void UTF8StringManipulator::setStr(const std::string & str)
{
	str_ = str;
	reset();
}

void UTF8StringManipulator::reset()
{
	iter_ = str_.begin();
}

wchar_t UTF8StringManipulator::next()
{
	if (!isAtEndOfStr())
		return utf8::next(iter_, str_.end());
	return '\0';
}

wchar_t UTF8StringManipulator::peekNext()
{
	if (isAtEndOfStr())
		return L'\0';

	return utf8::peek_next(iter_,str_.end());
}

wchar_t UTF8StringManipulator::peekPrevious()
{
	if (iter_ == str_.begin())
		return L'\0';

	auto tmpiter = iter_;
	return utf8::previous(tmpiter, str_.begin());
}

std::size_t UTF8StringManipulator::getSize() const
{
	return utf8::distance(str_.begin(), str_.end());
}

bool UTF8StringManipulator::isAtEndOfStr() const
{
	return iter_ == str_.end();
}
