////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : FTT.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "FTT/FTT.hpp"

using namespace fox::ftt;

bool Test::isDone() const
{
	return done_;
}

bool Test::hasPassed() const
{
	return isDone() ? passed_ : false;
}

Test::Test(std::ostream& os):
	out_(os),
	/* init bitfields */
	done_(false),
	passed_(false)
{
}

std::ostream& Test::out()
{
	return out_;
}

void Test::passed()
{
	done_ = true;
	passed_ = true;
}

void Test::failed()
{
	done_ = true;
	passed_ = false;
}

FileTest::FileTest(const std::string& file, std::ostream &os):
	Test(os)
{

}