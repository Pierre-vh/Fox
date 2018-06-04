////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : IdentifierTableTests.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//	(Unit) Tests for the SourceManager.
////------------------------------------------------------////

#include "gtest/gtest.h"
#include "TestUtils/TestUtils.hpp"

#include "Moonshot/Fox/Common/StringManipulator.hpp"
#include "Moonshot/Fox/Common/SourceManager.hpp"

using namespace Moonshot;

TEST(FileIDTests, UninitializedFileIDisInvalidFileID)
{
	EXPECT_FALSE(FileID()) << "Uninitialized File IDs should always be considered invalid!";
}

TEST(SourceManagerTests, LoadingFromFile)
{

}