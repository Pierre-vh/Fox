////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTTests.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//	(Unit) Tests for the AST nodes
////------------------------------------------------------////

#include "gtest/gtest.h"
#include "TestUtils/TestUtils.hpp"

#include "Moonshot/Fox/AST/ASTContext.hpp"
#include "Moonshot/Fox/AST/Types.hpp"

using namespace Moonshot;

// Tests isPrimitive, isArray, isBuiltin
TEST(ASTTests, TypeFunctions)
{
	ASTContext astContext;

	const Type* intTy = astContext.getPrimitiveIntType();
	ASSERT_TRUE(intTy) << "Ptr is null?";
	

	EXPECT_TRUE(intTy->isBuiltinType());
	EXPECT_TRUE(intTy->isPrimitiveType());
	EXPECT_FALSE(intTy->isArrayType());

	std::unique_ptr<Type>  arrInt = std::make_unique<ArrayType>(intTy);
	EXPECT_TRUE(arrInt->isArrayType());
	EXPECT_TRUE(arrInt->isBuiltinType());
	EXPECT_FALSE(arrInt->isPrimitiveType());
}

// Tests that the ASTContext gives correct builtin
TEST(ASTTests, ASTContextBuiltins)
{
	ASTContext actxt;

	auto primBool	= actxt.getPrimitiveBoolType();
	auto primFloat	= actxt.getPrimitiveFloatType();
	auto primInt	= actxt.getPrimitiveIntType();
	auto primChar	= actxt.getPrimitiveCharType();
	auto primString = actxt.getPrimitiveStringType();
	auto primVoid	= actxt.getPrimitiveVoidType();

	ASSERT_TRUE(primBool)	<< "Ptr is null?";
	ASSERT_TRUE(primFloat)	<< "Ptr is null?";
	ASSERT_TRUE(primInt)	<< "Ptr is null?";
	ASSERT_TRUE(primChar)	<< "Ptr is null?";
	ASSERT_TRUE(primString) << "Ptr is null?";
	ASSERT_TRUE(primVoid)	<< "Ptr is null?";

	// Checks that they're all different
	EXPECT_NE(primBool, primFloat);
	EXPECT_NE(primFloat, primInt);
	EXPECT_NE(primInt, primChar);
	EXPECT_NE(primChar, primString);
	EXPECT_NE(primString, primVoid);

	// Test that the types have the correct properties
	// Bools
	EXPECT_EQ(primBool->getBuiltinKind(),	PrimitiveType::Kind::BoolTy);
	EXPECT_TRUE(primBool->isArithmetic());
	EXPECT_FALSE(primBool->isConcatenable());
	EXPECT_FALSE(primBool->isVoid());

	// Floats
	EXPECT_EQ(primFloat->getBuiltinKind(),	PrimitiveType::Kind::FloatTy);
	EXPECT_TRUE(primFloat->isArithmetic());
	EXPECT_FALSE(primFloat->isConcatenable());
	EXPECT_FALSE(primFloat->isVoid());

	// Ints
	EXPECT_EQ(primInt->getBuiltinKind(),	PrimitiveType::Kind::IntTy);
	EXPECT_TRUE(primInt->isArithmetic());
	EXPECT_FALSE(primInt->isConcatenable());
	EXPECT_FALSE(primInt->isVoid());

	// Chars
	EXPECT_EQ(primChar->getBuiltinKind(), PrimitiveType::Kind::CharTy);
	EXPECT_TRUE(primChar->isConcatenable());
	EXPECT_FALSE(primChar->isArithmetic());
	EXPECT_FALSE(primChar->isVoid());

	// Strings
	EXPECT_EQ(primString->getBuiltinKind(), PrimitiveType::Kind::StringTy);
	EXPECT_TRUE(primString->isConcatenable());
	EXPECT_FALSE(primString->isArithmetic());
	EXPECT_FALSE(primString->isVoid());

	// Void type
	EXPECT_EQ(primVoid->getBuiltinKind(),	PrimitiveType::Kind::VoidTy);
	EXPECT_TRUE(primVoid->isVoid());
	EXPECT_FALSE(primVoid->isConcatenable());
	EXPECT_FALSE(primVoid->isArithmetic());
}

TEST(ASTTests, ASTContextArrayTypes)
{
	ASTContext actxt;

	auto primBool = actxt.getPrimitiveBoolType();
	auto primFloat = actxt.getPrimitiveFloatType();
	auto primInt = actxt.getPrimitiveIntType();
	auto primChar = actxt.getPrimitiveCharType();
	auto primString = actxt.getPrimitiveStringType();

	auto boolArr	= actxt.getArrayTypeForType(primBool);
	auto floatArr	= actxt.getArrayTypeForType(primFloat);
	auto intArr		= actxt.getArrayTypeForType(primInt);
	auto charArr	= actxt.getArrayTypeForType(primChar);
	auto strArr		= actxt.getArrayTypeForType(primString);


	// Check that pointers aren't null
	ASSERT_TRUE(boolArr)	<< "Pointer is null";
	ASSERT_TRUE(floatArr)	<< "Pointer is null";
	ASSERT_TRUE(intArr)		<< "Pointer is null";
	ASSERT_TRUE(charArr)	<< "Pointer is null";
	ASSERT_TRUE(strArr)		<< "Pointer is null";

	// Check that itemTypes are correct
	EXPECT_EQ(boolArr->getItemTy(), primBool);
	EXPECT_EQ(floatArr->getItemTy(), primFloat);
	EXPECT_EQ(intArr->getItemTy(), primInt);
	EXPECT_EQ(charArr->getItemTy(), primChar);
	EXPECT_EQ(strArr->getItemTy(), primString);

	// Checks that they're different
	EXPECT_NE(boolArr, floatArr);
	EXPECT_NE(floatArr, intArr);
	EXPECT_NE(intArr, charArr);
	EXPECT_NE(charArr, strArr);

	// Check that uniqueness works by getting the arraytype for int 100 times.
	// Everytime the pointer is returned it must be equal to intArr
	for (unsigned k = 0; k < 100; k++)
	{
		EXPECT_EQ(actxt.getArrayTypeForType(primInt), intArr);
	}
}