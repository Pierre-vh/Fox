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
#include "Fox/AST/Decl.hpp"
#include "Fox/AST/Expr.hpp"
#include "Fox/AST/ASTContext.hpp"
#include "Fox/AST/Types.hpp"
#include "Fox/AST/ASTVisitor.hpp"
#include "Fox/Common/LLVM.hpp"

using namespace fox;

// Tests that the ASTContext gives correct builtin
TEST(ASTTests, ASTContextBuiltins)
{
	ASTContext actxt;

	auto primBool	= actxt.getBoolType();
	auto primFloat	= actxt.getFloatType();
	auto primInt	= actxt.getIntType();
	auto primChar	= actxt.getCharType();
	auto primString = actxt.getStringType();
	auto primVoid	= actxt.getVoidType();

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
	EXPECT_EQ(primBool->getPrimitiveKind(),	PrimitiveType::Kind::BoolTy);
	EXPECT_TRUE(primBool->isBool());
	EXPECT_FALSE(primBool->isVoid());

	// Floats
	EXPECT_EQ(primFloat->getPrimitiveKind(),	PrimitiveType::Kind::FloatTy);
	EXPECT_TRUE(primFloat->isFloat());
	EXPECT_FALSE(primFloat->isVoid());

	// Ints
	EXPECT_EQ(primInt->getPrimitiveKind(),	PrimitiveType::Kind::IntTy);
	EXPECT_TRUE(primInt->isInt());
	EXPECT_FALSE(primInt->isVoid());

	// Chars
	EXPECT_EQ(primChar->getPrimitiveKind(), PrimitiveType::Kind::CharTy);
	EXPECT_TRUE(primChar->isChar());
	EXPECT_FALSE(primChar->isVoid());

	// Strings
	EXPECT_EQ(primString->getPrimitiveKind(), PrimitiveType::Kind::StringTy);
	EXPECT_TRUE(primString->isString());
	EXPECT_FALSE(primString->isVoid());

	// Void type
	EXPECT_EQ(primVoid->getPrimitiveKind(),	PrimitiveType::Kind::VoidTy);
	EXPECT_TRUE(primVoid->isVoid());
}

TEST(ASTTests, ASTContextArrayTypes)
{
	ASTContext actxt;

	TypeBase* primBool = actxt.getBoolType();
	TypeBase* primFloat = actxt.getFloatType();
	TypeBase* primInt = actxt.getIntType();
	TypeBase* primChar = actxt.getCharType();
	TypeBase* primString = actxt.getStringType();

	TypeBase* boolArr = actxt.getArrayTypeForType(primBool);
	TypeBase* floatArr = actxt.getArrayTypeForType(primFloat);
	TypeBase* intArr = actxt.getArrayTypeForType(primInt);
	TypeBase* charArr = actxt.getArrayTypeForType(primChar);
	TypeBase* strArr = actxt.getArrayTypeForType(primString);


	// Check that pointers aren't null
	ASSERT_TRUE(boolArr)	<< "Pointer is null";
	ASSERT_TRUE(floatArr)	<< "Pointer is null";
	ASSERT_TRUE(intArr)		<< "Pointer is null";
	ASSERT_TRUE(charArr)	<< "Pointer is null";
	ASSERT_TRUE(strArr)		<< "Pointer is null";

	// Check that itemTypes are correct
	EXPECT_EQ((dyn_cast<ArrayType>(boolArr))->getElementType(), primBool);
	EXPECT_EQ((dyn_cast<ArrayType>(floatArr))->getElementType(), primFloat);
	EXPECT_EQ((dyn_cast<ArrayType>(intArr))->getElementType(), primInt);
	EXPECT_EQ((dyn_cast<ArrayType>(charArr))->getElementType(), primChar);
	EXPECT_EQ((dyn_cast<ArrayType>(strArr))->getElementType(), primString);

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
VarDecl* makeVarDecl(ASTContext& ctxt, const std::string &name, TypeBase* ty)
{
	return new(ctxt) VarDecl(
			ctxt.identifiers.getUniqueIdentifierInfo(name),
			nullptr,
			false,
			nullptr,
			SourceRange(),
			SourceRange()
		);
}

FuncDecl* makeFuncDecl(ASTContext& ctxt, const std::string& name)
{
	return new(ctxt) FuncDecl(
		ctxt.getVoidType(),
		ctxt.identifiers.getUniqueIdentifierInfo(name),
		nullptr,
		SourceRange(),
		SourceLoc()
	);
}

bool testLookup(ASTContext &ctxt,DeclContext *dr, const std::string& name, Decl* decl,std::string& err)
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

TEST(ASTTests, DeclContext)
{
	ASTContext astctxt;

	auto* var1 = makeVarDecl(astctxt, "Variable_1", astctxt.getBoolType());
	auto* var2 = makeVarDecl(astctxt, "Variable_2", astctxt.getCharType());
	auto* var3 = makeVarDecl(astctxt, "Variable_3", astctxt.getFloatType());
	auto* var4 = makeVarDecl(astctxt, "Variable_4", astctxt.getStringType());
	auto* var5 = makeVarDecl(astctxt, "Variable_5", astctxt.getIntType());

	auto* func = makeFuncDecl(astctxt, "Foo");

	func->recordDecl(var1);
	func->recordDecl(var2);
	func->recordDecl(var3);
	func->recordDecl(var4);
	func->recordDecl(var5);

	// Iterate over all the recorded decl and check that they were added in the correct order with the correct names.
	bool v1_ok, v2_ok, v3_ok, v4_ok, v5_ok;
	v1_ok = v2_ok = v3_ok = v4_ok = v5_ok = false;
	for (auto it = func->recordedDecls_begin(); it != func->recordedDecls_end(); it++)
	{
		Identifier* id = it->getIdentifier();
		std::string str = id->getStr();
		if (str == "Variable_1")
		{
			EXPECT_EQ(id, var1->getIdentifier()) << "Mismatch : " << str << " != " << var1->getIdentifier()->getStr();
			EXPECT_EQ(*it, var1);
			ASSERT_FALSE(v1_ok) << "Variable_1 found twice?";
			v1_ok = true;
		}
		else if (str == "Variable_2")
		{
			EXPECT_EQ(id, var2->getIdentifier()) << "Mismatch : " << str << " != " << var2->getIdentifier()->getStr();
			EXPECT_EQ(*it, var2);
			ASSERT_FALSE(v2_ok) << "Variable_2 found twice?";
			v2_ok = true;
		}
		else if (str == "Variable_3")
		{
			EXPECT_EQ(id, var3->getIdentifier()) << "Mismatch : " << str << " != " << var3->getIdentifier()->getStr();
			EXPECT_EQ(*it, var3);
			ASSERT_FALSE(v3_ok) << "Variable_3 found twice?";
			v3_ok = true;
		}
		else if (str == "Variable_4")
		{
			EXPECT_EQ(id, var4->getIdentifier()) << "Mismatch : " << str << " != " << var4->getIdentifier()->getStr();
			EXPECT_EQ(*it, var4);
			ASSERT_FALSE(v4_ok) << "Variable_4 found twice?";
			v4_ok = true;
		}
		else if (str == "Variable_5")
		{
			EXPECT_EQ(id, var5->getIdentifier()) << "Mismatch : " << str << " != " << var5->getIdentifier()->getStr();
			EXPECT_EQ(*it, var5);
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
	EXPECT_TRUE(testLookup(astctxt, func, "Variable_1", var1, lasterr)) << lasterr;
	EXPECT_TRUE(testLookup(astctxt, func, "Variable_2", var2, lasterr)) << lasterr;
	EXPECT_TRUE(testLookup(astctxt, func, "Variable_3", var3, lasterr)) << lasterr;
	EXPECT_TRUE(testLookup(astctxt, func, "Variable_4", var4, lasterr)) << lasterr;
	EXPECT_TRUE(testLookup(astctxt, func, "Variable_5", var5, lasterr)) << lasterr;

	// Bad lookup tests
	EXPECT_FALSE(testLookup(astctxt, func, "Variable_6", var5, lasterr)) << lasterr;

}

TEST(ASTTests, TypeRTTI)
{
	ASTContext astctxt;
	TypeBase* intTy = astctxt.getIntType();
	ArrayType* arrIntTy = astctxt.getArrayTypeForType(intTy);
	LValueType* lvIntTy = astctxt.getLValueTypeForType(intTy);
	SemaType* semaType = astctxt.createSemaType();
	ErrorType* errType = astctxt.getErrorType();

	EXPECT_EQ(intTy->getKind(), TypeKind::PrimitiveType);
	EXPECT_TRUE(PrimitiveType::classof(intTy));
	EXPECT_TRUE(BuiltinType::classof(intTy));

	EXPECT_EQ(arrIntTy->getKind(), TypeKind::ArrayType);
	EXPECT_TRUE(ArrayType::classof(arrIntTy));
	EXPECT_TRUE(BuiltinType::classof(arrIntTy));

	EXPECT_EQ(lvIntTy->getKind(), TypeKind::LValueType);
	EXPECT_TRUE(LValueType::classof(lvIntTy));

	EXPECT_EQ(semaType->getKind(), TypeKind::SemaType);
	EXPECT_TRUE(SemaType::classof(semaType));

	EXPECT_EQ(errType->getKind(), TypeKind::ErrorType);
	EXPECT_TRUE(ErrorType::classof(errType));
}

TEST(ASTTests, ExprRTTI)
{
	ASTContext astctxt;

	// Binary Exprs
	BinaryExpr binexpr;
	EXPECT_EQ(binexpr.getKind(), ExprKind::BinaryExpr);
	EXPECT_TRUE(BinaryExpr::classof(&binexpr));

	// Unary Exprs
	UnaryExpr unaryexpr;
	EXPECT_EQ(unaryexpr.getKind(), ExprKind::UnaryExpr);
	EXPECT_TRUE(UnaryExpr::classof(&unaryexpr));

	// Cast Exprs
	CastExpr castexpr;
	EXPECT_EQ(castexpr.getKind(), ExprKind::CastExpr);
	EXPECT_TRUE(CastExpr::classof(&castexpr));

	// Literals
	CharLiteralExpr	charlit;
	EXPECT_EQ(charlit.getKind(), ExprKind::CharLiteralExpr);
	EXPECT_TRUE(CharLiteralExpr::classof(&charlit));

	IntegerLiteralExpr intlit;
	EXPECT_EQ(intlit.getKind(), ExprKind::IntegerLiteralExpr);
	EXPECT_TRUE(IntegerLiteralExpr::classof(&intlit));

	FloatLiteralExpr floatlit;
	EXPECT_EQ(floatlit.getKind(), ExprKind::FloatLiteralExpr);
	EXPECT_TRUE(FloatLiteralExpr::classof(&floatlit));

	StringLiteralExpr strlit;
	EXPECT_EQ(strlit.getKind(), ExprKind::StringLiteralExpr);
	EXPECT_TRUE(StringLiteralExpr::classof(&strlit));

	BoolLiteralExpr	boollit;
	EXPECT_EQ(boollit.getKind(), ExprKind::BoolLiteralExpr);
	EXPECT_TRUE(BoolLiteralExpr::classof(&boollit));

	ArrayLiteralExpr arrlit;
	EXPECT_EQ(arrlit.getKind(), ExprKind::ArrayLiteralExpr);
	EXPECT_TRUE(ArrayLiteralExpr::classof(&arrlit));

	// Helper
	auto fooid = astctxt.identifiers.getUniqueIdentifierInfo("foo");

	// DeclRef
	DeclRefExpr declref;
	EXPECT_EQ(declref.getKind(), ExprKind::DeclRefExpr);
	EXPECT_TRUE(DeclRefExpr::classof(&declref));

	// MemberOfExpr
	MemberOfExpr membof;
	EXPECT_EQ(membof.getKind(), ExprKind::MemberOfExpr);
	EXPECT_TRUE(MemberOfExpr::classof(&membof));

	// Array Access
	ArrayAccessExpr arracc;
	EXPECT_EQ(arracc.getKind(), ExprKind::ArrayAccessExpr);
	EXPECT_TRUE(ArrayAccessExpr::classof(&arracc));

	// Function calls
	FunctionCallExpr callexpr;
	EXPECT_EQ(callexpr.getKind(), ExprKind::FunctionCallExpr);
	EXPECT_TRUE(FunctionCallExpr::classof(&callexpr));
}

TEST(ASTTests, StmtRTTI)
{
	// NullStmt
	NullStmt null;
	EXPECT_EQ(null.getKind(), StmtKind::NullStmt);
	EXPECT_TRUE(NullStmt::classof(&null));

	// Return stmt
	ReturnStmt rtr;
	EXPECT_EQ(rtr.getKind(), StmtKind::ReturnStmt);
	EXPECT_TRUE(ReturnStmt::classof(&rtr));

	// Condition
	ConditionStmt cond;
	EXPECT_EQ(cond.getKind(), StmtKind::ConditionStmt);
	EXPECT_TRUE(ConditionStmt::classof(&cond));

	// Compound
	CompoundStmt compound;
	EXPECT_EQ(compound.getKind(), StmtKind::CompoundStmt);
	EXPECT_TRUE(CompoundStmt::classof(&compound));

	// While
	WhileStmt whilestmt;
	EXPECT_EQ(whilestmt.getKind(), StmtKind::WhileStmt);
	EXPECT_TRUE(WhileStmt::classof(&whilestmt));
}

TEST(ASTTests, DeclRTTI)
{
	ASTContext astctxt;
	auto fooid = astctxt.identifiers.getUniqueIdentifierInfo("foo");
	auto intty = astctxt.getIntType();

	// Arg
	ParamDecl paramdecl;
	EXPECT_EQ(paramdecl.getKind(), DeclKind::ParamDecl);
	EXPECT_TRUE(ParamDecl::classof(&paramdecl));
	EXPECT_TRUE(NamedDecl::classof(&paramdecl));
	EXPECT_TRUE(ValueDecl::classof(&paramdecl));
	EXPECT_FALSE(DeclContext::classof(&paramdecl));

	// Func
	FuncDecl fndecl;
	EXPECT_EQ(fndecl.getKind(), DeclKind::FuncDecl);
	EXPECT_TRUE(FuncDecl::classof(&fndecl));
	EXPECT_TRUE(NamedDecl::classof(&fndecl));
	EXPECT_TRUE(DeclContext::classof(&fndecl));

	// Var
	VarDecl vdecl;
	EXPECT_EQ(vdecl.getKind(), DeclKind::VarDecl);
	EXPECT_TRUE(VarDecl::classof(&vdecl));
	EXPECT_TRUE(NamedDecl::classof(&vdecl));
	EXPECT_TRUE(ValueDecl::classof(&vdecl));
	EXPECT_FALSE(DeclContext::classof(&vdecl));

	// Unit
	UnitDecl udecl(fooid,FileID());
	EXPECT_EQ(udecl.getKind(), DeclKind::UnitDecl);
	EXPECT_TRUE(UnitDecl::classof(&udecl));
	EXPECT_TRUE(NamedDecl::classof(&udecl));
	EXPECT_TRUE(DeclContext::classof(&udecl));
}

// ASTVisitor tests : Samples implementations to test if visitors works as intended
class IsNamedDecl : public SimpleASTVisitor<IsNamedDecl, bool>
{
	public:
		bool visitNamedDecl(NamedDecl* node)
		{
			return true;
		}
};

class IsExpr : public SimpleASTVisitor<IsExpr, bool>
{
	public:
		bool visitExpr(Expr* node)
		{
			return true;
		}
};

class IsArrTy : public SimpleASTVisitor<IsArrTy, bool>
{
	public:
		bool visitArrayType(ArrayType* node)
		{
			return true;
		}
};

TEST(ASTTests, BasicVisitor)
{
	// Context
	ASTContext ctxt;

	// Create test nodes
	auto* intlit = new(ctxt) IntegerLiteralExpr(200, SourceRange());
	auto* rtr = new(ctxt) ReturnStmt(nullptr, SourceRange());
	auto* vardecl = new(ctxt) VarDecl(
			nullptr,
			nullptr,
			false,
			nullptr, 
			SourceRange(),
			SourceRange()
		);
	auto intTy = ctxt.getIntType();
	auto arrInt = ctxt.getArrayTypeForType(intTy);

	IsExpr exprVisitor;
	IsNamedDecl declVisitor;
	IsArrTy tyVisitor;

	EXPECT_TRUE(exprVisitor.visit(intlit));
	EXPECT_FALSE(exprVisitor.visit(rtr));
	EXPECT_FALSE(exprVisitor.visit(vardecl));
	EXPECT_FALSE(exprVisitor.visit(intTy));
	EXPECT_FALSE(exprVisitor.visit(arrInt));

	EXPECT_FALSE(declVisitor.visit(intlit));
	EXPECT_FALSE(declVisitor.visit(rtr));
	EXPECT_TRUE(declVisitor.visit(vardecl));
	EXPECT_FALSE(declVisitor.visit(intTy));
	EXPECT_FALSE(declVisitor.visit(arrInt));

	EXPECT_FALSE(tyVisitor.visit(intlit));
	EXPECT_FALSE(tyVisitor.visit(rtr));
	EXPECT_FALSE(tyVisitor.visit(vardecl));
	EXPECT_FALSE(tyVisitor.visit(intTy));
	EXPECT_TRUE(tyVisitor.visit(arrInt));

}