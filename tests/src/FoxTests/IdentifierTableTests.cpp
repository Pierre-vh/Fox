////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : IdentifierTableTests.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//	(Unit) Tests for the IdentifierTable.
////------------------------------------------------------////

#include "gtest/gtest.h"
#include "Support/TestUtils.hpp"

#include "Fox/AST/Identifiers.hpp"

#include <algorithm>
#include <string>
#include <random>

using namespace fox;

// Number of identifiers to insert into the table in the "randomIdentifierInsertion" test.
#define RANDOM_ID_TEST_NUMBER_OF_ID 2048
// Note, if theses values are too low, the test might fail sometimes because there's a change that the randomly generated
// identifier is already taken. Using high values make the test longer, but also a lot less unlikely to fail!
#define RANDOM_STRING_MIN_LENGTH 128
#define RANDOM_STRING_MAX_LENGTH 128

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
	Identifier* idA = idtab.getUniqueIdentifierInfo(rawIdA);
	Identifier* idB = idtab.getUniqueIdentifierInfo(rawIdB);

	ASSERT_NE(idA, idB);
	ASSERT_NE(idA->getStr(), idB->getStr()) << "The 2 strings are not the same!";
}

// Checks if the exists function works correctly.
TEST(IdentifierTableTests, exists)
{
	std::string randID = generateRandomString();

	IdentifierTable idtab;

	EXPECT_FALSE(idtab.exists(randID));

	idtab.getUniqueIdentifierInfo(randID);

	EXPECT_TRUE(idtab.exists(randID));
}

// Checks if the IdentifierTable supports large identifiers amount by inserting a lot of random ids.
TEST(IdentifierTableTests, randomIdentifierInsertion)
{
	IdentifierTable idtab;
	std::string id;

	Identifier *ptr = nullptr; // Last IdInfo's adress
	
	std::vector<Identifier*> alldIdInfoPtrs;
	std::vector<std::string> allIdStrs;

	for (std::size_t k(0); k < RANDOM_ID_TEST_NUMBER_OF_ID; k++)
	{
		id = generateRandomString();
		
		// Before inserting, a quick sanity check doesn't hurt!
		ASSERT_FALSE(idtab.exists(id)) << "[Insertion " << k << "] The identifier \"" << id << "\" already exists";
		
		auto idinfo = idtab.getUniqueIdentifierInfo(id);
		// Check if the string matches, and if the adress of this type is different from the last one used.
		ASSERT_EQ(idinfo->getStr(), id) << "[Insertion " << k << "] Strings did not match";
		ASSERT_TRUE(idtab.exists(id)) << "[Insertion " << k << "] IdentifierTable is reporting that the identifier does not exists.";
		ASSERT_NE(ptr,idinfo) << "[Insertion " << k << "] Insertion returned a already in use pointer.";
		
		ptr = idinfo;
		
		allIdStrs.push_back(id);
		alldIdInfoPtrs.push_back(idinfo);
	}

	// Now, iterate over all identifierinfo to check if they're still valid even after that much insertions.
	for (std::size_t idx(0);idx < alldIdInfoPtrs.size(); idx++)
	{
		ASSERT_TRUE(alldIdInfoPtrs[idx] != nullptr) << "Pointer was null?";
		ASSERT_EQ(
			allIdStrs[idx],
			alldIdInfoPtrs[idx]->getStr()
		) << "Bad identifierInfo?";
	}
}

static const std::string idStrChars = "_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

std::string generateRandomString()
{
	std::random_device rd;
	std::mt19937_64 mt(rd());
	std::uniform_int_distribution<int> dist_char(0, (int)idStrChars.size());

	std::uniform_int_distribution<int> dist_length(RANDOM_STRING_MIN_LENGTH, RANDOM_STRING_MAX_LENGTH);
	int strlen = dist_length(mt);

	std::string output;
	std::generate_n(std::back_inserter(output), strlen, [&] {return idStrChars[dist_char(mt)]; });
	return output;
}