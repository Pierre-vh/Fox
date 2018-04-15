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
TEST(ASTTests, BaseTypeFunctions)
{
	ASTContext astContext;

	Type* intTy = astContext.getPrimitiveIntType();
	ASSERT_TRUE(intTy) << "Ptr is null?";
	

	EXPECT_TRUE(intTy->isBuiltinType()) << "Primitive does not register as builtin";
	EXPECT_TRUE(intTy->isPrimitiveType()) << "Primitive does not register as primitive";

	std::unique_ptr<Type>  arrInt = std::make_unique<ArrayType>(intTy);
	EXPECT_TRUE(arrInt->isArrayType()) << "Array does not register as array";
}

// Tests that the ASTContext gives correct builtin
TEST(ASTTests, ASTContextBuiltins)
{
	ASTContext actxt;

	PrimitiveType*  primBool	= actxt.getPrimitiveBoolType();
	PrimitiveType*  primFloat	= actxt.getPrimitiveFloatType();
	PrimitiveType*  primInt		= actxt.getPrimitiveIntType();
	PrimitiveType*  primChar	= actxt.getPrimitiveCharType();
	PrimitiveType*  primString	= actxt.getPrimitiveStringType();
	PrimitiveType*  primVoid	= actxt.getPrimitiveVoidType();

	ASSERT_TRUE(primBool)	<< "Ptr is null?";
	ASSERT_TRUE(primFloat)	<< "Ptr is null?";
	ASSERT_TRUE(primInt)	<< "Ptr is null?";
	ASSERT_TRUE(primChar)	<< "Ptr is null?";
	ASSERT_TRUE(primString) << "Ptr is null?";
	ASSERT_TRUE(primVoid)	<< "Ptr is null?";


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