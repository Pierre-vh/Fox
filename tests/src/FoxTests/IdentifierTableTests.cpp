////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : IdentifierTableTests.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//	(Unit) Tests for the IdentifierTable.
////------------------------------------------------------////

#include "gtest/gtest.h"
#include "TestUtils/TestUtils.hpp"

#include "Moonshot/Fox/AST/IdentifierTable.hpp"

#include <algorithm>
#include <string>
#include <random>

using namespace Moonshot;

// Number of identifiers to insert into the table in the "randomIdentifierInsertion" test.
#define RANDOM_ID_TEST_NUMBER_OF_ID 4200

std::string generateRandomString();

// Checks if Identifiers are unique, or not.
TEST(IdentifierTableTests, areIdentifiersUnique)
{
	// Create 2 identifiers, A and B
	std::string rawIdA, rawIdB;
	rawIdA = generateRandomString();
	rawIdB = generateRandomString();
	ASSERT_NE(rawIdA, rawIdB) << "The 2 randomly generated identifiers were the same ! Is the generator function broken?";

	IdentifierTable idtab;
	IdentifierInfo& idA = idtab.getUniqueIDinfo(rawIdA);
	IdentifierInfo& idB = idtab.getUniqueIDinfo(rawIdB);

	ASSERT_NE(&idA, &idB) << "The 2 references point to the same IdentifierInfo!";
	ASSERT_NE(idA.getStr(), idB.getStr()) << "The 2 strings are not the same!";

	// try to get them again and compare again
	IdentifierInfo& idA_2 = idtab.getUniqueIDinfo(rawIdA);
	IdentifierInfo& idB_2 = idtab.getUniqueIDinfo(rawIdB);

	EXPECT_NE(&idA_2, &idB_2) << "The 2 references point to the same IdentifierInfo!";
	EXPECT_NE(idA_2.getStr(), idB_2.getStr()) << "The 2 strings are not the same!";

	EXPECT_EQ(&idA, &idA_2) << "The IdentifierTable did not return the expected IdentifierInfo!";
	EXPECT_EQ(idA.getStr(),idA_2.getStr()) << "The IdentifierInfo did not return the expected string!";

	EXPECT_EQ(&idB, &idB_2) << "The IdentifierTable did not return the expected IdentifierInfo!";
	EXPECT_EQ(idB.getStr(), idB_2.getStr()) << "The IdentifierInfo did not return the expected string!";
}

// Checks if the exists function works correctly.
TEST(IdentifierTableTests, exists)
{
	std::string randID = generateRandomString();

	IdentifierTable idtab;

	EXPECT_FALSE(idtab.exists(randID));

	idtab.getUniqueIDinfo(randID);

	EXPECT_TRUE(idtab.exists(randID));
}

// Checks if the IdentifierTable supports large identifiers amount by inserting a lot of random ids.
TEST(IdentifierTableTests, randomIdentifierInsertion)
{
	IdentifierTable idtab;
	std::string id;

	IdentifierInfo *ptr = nullptr;
	for (std::size_t k(0); k < RANDOM_ID_TEST_NUMBER_OF_ID; k++)
	{
		id = generateRandomString();

		// Before inserting, a quick sanity check doesn't hurt!
		ASSERT_FALSE(idtab.exists(id)) << "[Insertion " << k << "] The identifier \"" << id << "\" already exists";
		
		IdentifierInfo &idinfo = idtab.getUniqueIDinfo(id);
		// Check if the string matches, and if the adress of this type is different from the last one used.
		ASSERT_EQ(idinfo.getStr(), id) << "[Insertion " << k << "] Strings did not match";
		ASSERT_TRUE(idtab.exists(id)) << "[Insertion " << k << "] IdentifierTable is reporting that the identifier does not exists.";
		ASSERT_NE(ptr, &idinfo) << "[Insertion " << k << "] Insertion returned a already in use pointer.";
		ptr = &idinfo;
	}
}

static const std::string idStrChars = "_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

std::string generateRandomString()
{
	std::random_device rd;
	std::mt19937_64 mt(rd());
	std::uniform_int_distribution<int> dist_char(0, idStrChars.size());

	std::uniform_int_distribution<int> dist_length(16, 64); // Generate strings between 16 and 64 characters.
	int strlen = dist_length(mt);

	std::string output;
	std::generate_n(std::back_inserter(output), strlen, [&] {return idStrChars[dist_char(mt)]; });
	return output;
}