////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTTests.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//	(Unit) Tests for the AST nodes
////------------------------------------------------------////

#include "gtest/gtest.h"
#include "Support/TestUtils.hpp"
#include "Moonshot/Fox/AST/Decl.hpp"
#include "Moonshot/Fox/AST/Expr.hpp"
#include "Moonshot/Fox/AST/ASTContext.hpp"
#include "Moonshot/Fox/AST/Type.hpp"
#include "Moonshot/Fox/AST/ASTVisitor.hpp"

using namespace Moonshot;

// Tests isPrimitive, isArray, isBuiltin
TEST(ASTTests, TypeFunctions)
{
	ASTContext astContext;

	Type* intTy = astContext.getPrimitiveIntType();
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

	Type* primBool = actxt.getPrimitiveBoolType();
	Type* primFloat = actxt.getPrimitiveFloatType();
	Type* primInt = actxt.getPrimitiveIntType();
	Type* primChar = actxt.getPrimitiveCharType();
	Type* primString = actxt.getPrimitiveStringType();

	Type* boolArr	= actxt.getArrayTypeForType(primBool);
	Type* floatArr	= actxt.getArrayTypeForType(primFloat);
	Type* intArr		= actxt.getArrayTypeForType(primInt);
	Type* charArr	= actxt.getArrayTypeForType(primChar);
	Type* strArr		= actxt.getArrayTypeForType(primString);


	// Check that pointers aren't null
	ASSERT_TRUE(boolArr)	<< "Pointer is null";
	ASSERT_TRUE(floatArr)	<< "Pointer is null";
	ASSERT_TRUE(intArr)		<< "Pointer is null";
	ASSERT_TRUE(charArr)	<< "Pointer is null";
	ASSERT_TRUE(strArr)		<< "Pointer is null";

	// Check that itemTypes are correct
	EXPECT_EQ((dynamic_cast<ArrayType*>(boolArr))->getItemTy(), primBool);
	EXPECT_EQ((dynamic_cast<ArrayType*>(floatArr))->getItemTy(), primFloat);
	EXPECT_EQ((dynamic_cast<ArrayType*>(intArr))->getItemTy(), primInt);
	EXPECT_EQ((dynamic_cast<ArrayType*>(charArr))->getItemTy(), primChar);
	EXPECT_EQ((dynamic_cast<ArrayType*>(strArr))->getItemTy(), primString);

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

// Create a variable with a random type
std::unique_ptr<VarDecl> makeVarDecl(ASTContext& ctxt, const std::string &name,Type* ty)
{
	return std::make_unique<VarDecl>(
			ctxt.identifiers.getUniqueIdentifierInfo(name),
			QualType(ty),
			nullptr,
			SourceLoc(),
			SourceRange(),
			SourceLoc()
		);
}

std::unique_ptr<FunctionDecl> makeFuncDecl(ASTContext& ctxt, const std::string& name)
{
	return std::make_unique<FunctionDecl>();
}

bool testLookup(ASTContext &ctxt,DeclRecorder *dr, const std::string& name, Decl* decl,std::string& err)
{
	auto lookupResult = dr->restrictedLookup(ctxt.identifiers.getUniqueIdentifierInfo(name));
	
	if (!lookupResult)
	{
		err = "No result found";
		return false;
	}

	if (!lookupResult.isUnique())
	{
		err = "Multiple results found";
		return false;
	}

	if (lookupResult.getResultIfUnique() == decl)
		return true;
	else
	{
		err = "Result isn't the one expected";
		return false;
	}
}

TEST(ASTTests, DeclRecorderTests)
{
	ASTContext astctxt;

	auto var1 = makeVarDecl(astctxt, "Variable_1", astctxt.getPrimitiveBoolType());
	auto var2 = makeVarDecl(astctxt, "Variable_2", astctxt.getPrimitiveCharType());
	auto var3 = makeVarDecl(astctxt, "Variable_3", astctxt.getPrimitiveFloatType());
	auto var4 = makeVarDecl(astctxt, "Variable_4", astctxt.getPrimitiveStringType());
	auto var5 = makeVarDecl(astctxt, "Variable_5", astctxt.getPrimitiveIntType());

	auto func = makeFuncDecl(astctxt, "Foo");

	func->recordDecl(var1.get());
	func->recordDecl(var2.get());
	func->recordDecl(var3.get());
	func->recordDecl(var4.get());
	func->recordDecl(var5.get());

	// Iterate over all the recorded decl and check that they were added in the correct order with the correct names.
	bool v1_ok, v2_ok, v3_ok, v4_ok, v5_ok;
	v1_ok = v2_ok = v3_ok = v4_ok = v5_ok = false;
	for (auto it = func->recordedDecls_begin(); it != func->recordedDecls_end(); it++)
	{
		IdentifierInfo* id = it->getIdentifier();
		std::string str = id->getStr();
		if (str == "Variable_1")
		{
			EXPECT_EQ(id, var1->getIdentifier()) << "Mismatch : " << str << " != " << var1->getIdentifier()->getStr();
			EXPECT_EQ(*it, var1.get());
			ASSERT_FALSE(v1_ok) << "Variable_1 found twice?";
			v1_ok = true;
		}
		else if (str == "Variable_2")
		{
			EXPECT_EQ(id, var2->getIdentifier()) << "Mismatch : " << str << " != " << var2->getIdentifier()->getStr();
			EXPECT_EQ(*it, var2.get());
			ASSERT_FALSE(v2_ok) << "Variable_2 found twice?";
			v2_ok = true;
		}
		else if (str == "Variable_3")
		{
			EXPECT_EQ(id, var3->getIdentifier()) << "Mismatch : " << str << " != " << var3->getIdentifier()->getStr();
			EXPECT_EQ(*it, var3.get());
			ASSERT_FALSE(v3_ok) << "Variable_3 found twice?";
			v3_ok = true;
		}
		else if (str == "Variable_4")
		{
			EXPECT_EQ(id, var4->getIdentifier()) << "Mismatch : " << str << " != " << var4->getIdentifier()->getStr();
			EXPECT_EQ(*it, var4.get());
			ASSERT_FALSE(v4_ok) << "Variable_4 found twice?";
			v4_ok = true;
		}
		else if (str == "Variable_5")
		{
			EXPECT_EQ(id, var5->getIdentifier()) << "Mismatch : " << str << " != " << var5->getIdentifier()->getStr();
			EXPECT_EQ(*it, var5.get());
			ASSERT_FALSE(v5_ok) << "Variable_5 found twice?";
			v5_ok = true;
		}
		else
		{
			FAIL() << "No decl found";
		}
	}
	EXPECT_TRUE(v1_ok && v2_ok && v3_ok && v4_ok && v5_ok) << "One or more variable was not found.";

	// Lookup tests
	std::string lasterr = "";
	EXPECT_TRUE(testLookup(astctxt, func.get(), "Variable_1", var1.get(), lasterr)) << lasterr;
	EXPECT_TRUE(testLookup(astctxt, func.get(), "Variable_2", var2.get(), lasterr)) << lasterr;
	EXPECT_TRUE(testLookup(astctxt, func.get(), "Variable_3", var3.get(), lasterr)) << lasterr;
	EXPECT_TRUE(testLookup(astctxt, func.get(), "Variable_4", var4.get(), lasterr)) << lasterr;
	EXPECT_TRUE(testLookup(astctxt, func.get(), "Variable_5", var5.get(), lasterr)) << lasterr;

	// Bad lookup tests
	EXPECT_FALSE(testLookup(astctxt, func.get(), "Variable_6", var5.get(), lasterr)) << lasterr;

}

TEST(ASTTests, TypeKinds)
{
	ASTContext astctxt;
	Type* intTy = astctxt.getPrimitiveIntType();
	Type* arrIntTy = astctxt.getArrayTypeForType(intTy);

	EXPECT_EQ(intTy->getKind(), TypeKind::PrimitiveType);
	EXPECT_EQ(arrIntTy->getKind(), TypeKind::ArrayType);
}

TEST(ASTTests, ExprKinds)
{
	ASTContext astctxt;

	// Binary Exprs
	BinaryExpr binexpr;
	EXPECT_EQ(binexpr.getKind(), StmtKind::BinaryExpr);

	// Unary Exprs
	UnaryExpr unaryexpr;
	EXPECT_EQ(unaryexpr.getKind(), StmtKind::UnaryExpr);

	// Cast Exprs
	CastExpr castexpr;
	EXPECT_EQ(castexpr.getKind(), StmtKind::CastExpr);

	// Literals
	CharLiteralExpr		charlit;
	IntegerLiteralExpr	intlit;
	FloatLiteralExpr	floatlit;
	StringLiteralExpr	strlit;
	BoolLiteralExpr		boollit;
	ArrayLiteralExpr	arrlit;

	EXPECT_EQ(charlit.getKind(), StmtKind::CharLiteralExpr);
	EXPECT_EQ(intlit.getKind(),  StmtKind::IntegerLiteralExpr);
	EXPECT_EQ(floatlit.getKind(),StmtKind::FloatLiteralExpr);
	EXPECT_EQ(strlit.getKind(),  StmtKind::StringLiteralExpr);
	EXPECT_EQ(boollit.getKind(), StmtKind::BoolLiteralExpr);
	EXPECT_EQ(arrlit.getKind(),  StmtKind::ArrayLiteralExpr);

	// Helper
	auto fooid = astctxt.identifiers.getUniqueIdentifierInfo("foo");

	// DeclRef
	DeclRefExpr declref;
	EXPECT_EQ(declref.getKind(), StmtKind::DeclRefExpr);

	// MemberOfExpr
	MemberOfExpr membof;
	EXPECT_EQ(membof.getKind(), StmtKind::MemberOfExpr);

	// Array Access
	ArrayAccessExpr arracc;
	EXPECT_EQ(arracc.getKind(), StmtKind::ArrayAccessExpr);

	// Function calls
	FunctionCallExpr callexpr;
	EXPECT_EQ(callexpr.getKind(), StmtKind::FunctionCallExpr);
}

TEST(ASTTests, StmtKinds)
{
	// NullStmt
	NullStmt null;
	EXPECT_EQ(null.getKind(), StmtKind::NullStmt);

	// Return stmt
	ReturnStmt rtr;
	EXPECT_EQ(rtr.getKind(), StmtKind::ReturnStmt);

	// Condition
	ConditionStmt cond;
	EXPECT_EQ(cond.getKind(), StmtKind::ConditionStmt);

	// Compound
	CompoundStmt compound;
	EXPECT_EQ(compound.getKind(), StmtKind::CompoundStmt);

	// While
	WhileStmt whilestmt;
	EXPECT_EQ(whilestmt.getKind(), StmtKind::WhileStmt);

	// declstmt
	DeclStmt declstmt(std::make_unique<FunctionDecl>()); /* The arg passed to a DeclStmt cannot be null */
	EXPECT_EQ(declstmt.getKind(), StmtKind::DeclStmt);
}

TEST(ASTTests, DeclKinds)
{
	ASTContext astctxt;
	auto fooid = astctxt.identifiers.getUniqueIdentifierInfo("foo");
	auto intty = astctxt.getPrimitiveIntType();

	// Arg
	ArgDecl argdecl;
	EXPECT_EQ(argdecl.getKind(), DeclKind::ArgDecl);

	// Func
	FunctionDecl fndecl;
	EXPECT_EQ(fndecl.getKind(), DeclKind::FunctionDecl);

	// Var
	VarDecl vdecl;
	EXPECT_EQ(vdecl.getKind(), DeclKind::VarDecl);

	// Unit
	UnitDecl udecl(fooid,FileID());
	EXPECT_EQ(udecl.getKind(), DeclKind::UnitDecl);
}

// ASTVisitor tests : Samples implementations to test if visitors works as intended
class IsNamedDecl : public ASTVisitor<IsNamedDecl, bool>
{
	public:
		bool visitNamedDecl(NamedDecl* node)
		{
			return true;
		}
};

class IsExpr : public ASTVisitor<IsExpr, bool>
{
	public:
		bool visitExpr(Expr* node)
		{
			return true;
		}
};

class IsArrTy : public ASTVisitor<IsArrTy, bool>
{
	public:
		bool visitArrayType(ArrayType* node)
		{
			return true;
		}
};

TEST(ASTTests, BasicVisitorTest)
{
	// Context
	ASTContext ctxt;

	// Create test nodes
	auto intlit = std::make_unique<IntegerLiteralExpr>(200,SourceLoc(),SourceLoc());
	auto rtr = std::make_unique<ReturnStmt>(nullptr,SourceLoc(),SourceLoc());
	auto vardecl = std::make_unique<VarDecl>(
			nullptr,
			QualType(),
			nullptr, 
			SourceLoc(),
			SourceRange(),
			SourceLoc()
		);
	auto intTy = ctxt.getPrimitiveIntType();
	auto arrInt = ctxt.getArrayTypeForType(intTy);

	IsExpr exprVisitor;
	IsNamedDecl declVisitor;
	IsArrTy tyVisitor;

	EXPECT_TRUE(exprVisitor.visit(intlit.get()));
	EXPECT_FALSE(exprVisitor.visit(rtr.get()));
	EXPECT_FALSE(exprVisitor.visit(vardecl.get()));
	EXPECT_FALSE(exprVisitor.visit(intTy));
	EXPECT_FALSE(exprVisitor.visit(arrInt));

	EXPECT_FALSE(declVisitor.visit(intlit.get()));
	EXPECT_FALSE(declVisitor.visit(rtr.get()));
	EXPECT_TRUE(declVisitor.visit(vardecl.get()));
	EXPECT_FALSE(declVisitor.visit(intTy));
	EXPECT_FALSE(declVisitor.visit(arrInt));

	EXPECT_FALSE(tyVisitor.visit(intlit.get()));
	EXPECT_FALSE(tyVisitor.visit(rtr.get()));
	EXPECT_FALSE(tyVisitor.visit(vardecl.get()));
	EXPECT_FALSE(tyVisitor.visit(intTy));
	EXPECT_TRUE(tyVisitor.visit(arrInt));

}