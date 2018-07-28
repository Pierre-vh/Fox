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
#include "Fox/AST/Type.hpp"
#include "Fox/AST/ASTVisitor.hpp"
#include "Fox/Common/LLVM.hpp"

using namespace fox;

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

	Type* boolArr = actxt.getArrayTypeForType(primBool);
	Type* floatArr = actxt.getArrayTypeForType(primFloat);
	Type* intArr = actxt.getArrayTypeForType(primInt);
	Type* charArr = actxt.getArrayTypeForType(primChar);
	Type* strArr = actxt.getArrayTypeForType(primString);


	// Check that pointers aren't null
	ASSERT_TRUE(boolArr)	<< "Pointer is null";
	ASSERT_TRUE(floatArr)	<< "Pointer is null";
	ASSERT_TRUE(intArr)		<< "Pointer is null";
	ASSERT_TRUE(charArr)	<< "Pointer is null";
	ASSERT_TRUE(strArr)		<< "Pointer is null";

	// Check that itemTypes are correct
	EXPECT_EQ((dyn_cast<ArrayType>(boolArr))->getItemTy(), primBool);
	EXPECT_EQ((dyn_cast<ArrayType>(floatArr))->getItemTy(), primFloat);
	EXPECT_EQ((dyn_cast<ArrayType>(intArr))->getItemTy(), primInt);
	EXPECT_EQ((dyn_cast<ArrayType>(charArr))->getItemTy(), primChar);
	EXPECT_EQ((dyn_cast<ArrayType>(strArr))->getItemTy(), primString);

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

std::unique_ptr<FuncDecl> makeFuncDecl(ASTContext& ctxt, const std::string& name)
{
	return std::make_unique<FuncDecl>();
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

TEST(ASTTests, DeclContextTest)
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

TEST(ASTTests, TypeRTTI)
{
	ASTContext astctxt;
	Type* intTy = astctxt.getPrimitiveIntType();
	Type* arrIntTy = astctxt.getArrayTypeForType(intTy);

	EXPECT_EQ(intTy->getKind(), TypeKind::PrimitiveType);
	EXPECT_TRUE(PrimitiveType::classof(intTy));
	EXPECT_TRUE(BuiltinType::classof(intTy));

	EXPECT_EQ(arrIntTy->getKind(), TypeKind::ArrayType);
	EXPECT_TRUE(ArrayType::classof(arrIntTy));
	EXPECT_TRUE(BuiltinType::classof(arrIntTy));
}

TEST(ASTTests, ExprRTTI)
{
	ASTContext astctxt;

	// Binary Exprs
	BinaryExpr binexpr;
	EXPECT_EQ(binexpr.getKind(), StmtKind::BinaryExpr);
	EXPECT_TRUE(Expr::classof(&binexpr));
	EXPECT_TRUE(BinaryExpr::classof(&binexpr));

	// Unary Exprs
	UnaryExpr unaryexpr;
	EXPECT_EQ(unaryexpr.getKind(), StmtKind::UnaryExpr);
	EXPECT_TRUE(Expr::classof(&unaryexpr));
	EXPECT_TRUE(UnaryExpr::classof(&unaryexpr));

	// Cast Exprs
	CastExpr castexpr;
	EXPECT_EQ(castexpr.getKind(), StmtKind::CastExpr);
	EXPECT_TRUE(Expr::classof(&castexpr));
	EXPECT_TRUE(CastExpr::classof(&castexpr));

	// Literals
	CharLiteralExpr	charlit;
	EXPECT_EQ(charlit.getKind(), StmtKind::CharLiteralExpr);
	EXPECT_TRUE(Expr::classof(&charlit));
	EXPECT_TRUE(CharLiteralExpr::classof(&charlit));

	IntegerLiteralExpr intlit;
	EXPECT_EQ(intlit.getKind(),  StmtKind::IntegerLiteralExpr);
	EXPECT_TRUE(Expr::classof(&intlit));
	EXPECT_TRUE(IntegerLiteralExpr::classof(&intlit));

	FloatLiteralExpr floatlit;
	EXPECT_EQ(floatlit.getKind(),StmtKind::FloatLiteralExpr);
	EXPECT_TRUE(Expr::classof(&floatlit));
	EXPECT_TRUE(FloatLiteralExpr::classof(&floatlit));

	StringLiteralExpr strlit;
	EXPECT_EQ(strlit.getKind(),  StmtKind::StringLiteralExpr);
	EXPECT_TRUE(Expr::classof(&strlit));
	EXPECT_TRUE(StringLiteralExpr::classof(&strlit));

	BoolLiteralExpr	boollit;
	EXPECT_EQ(boollit.getKind(), StmtKind::BoolLiteralExpr);
	EXPECT_TRUE(Expr::classof(&boollit));
	EXPECT_TRUE(BoolLiteralExpr::classof(&boollit));

	ArrayLiteralExpr arrlit;
	EXPECT_EQ(arrlit.getKind(),  StmtKind::ArrayLiteralExpr);
	EXPECT_TRUE(Expr::classof(&arrlit));
	EXPECT_TRUE(ArrayLiteralExpr::classof(&arrlit));

	// Helper
	auto fooid = astctxt.identifiers.getUniqueIdentifierInfo("foo");

	// DeclRef
	DeclRefExpr declref;
	EXPECT_EQ(declref.getKind(), StmtKind::DeclRefExpr);
	EXPECT_TRUE(Expr::classof(&declref));
	EXPECT_TRUE(DeclRefExpr::classof(&declref));

	// MemberOfExpr
	MemberOfExpr membof;
	EXPECT_EQ(membof.getKind(), StmtKind::MemberOfExpr);
	EXPECT_TRUE(Expr::classof(&membof));
	EXPECT_TRUE(MemberOfExpr::classof(&membof));

	// Array Access
	ArrayAccessExpr arracc;
	EXPECT_EQ(arracc.getKind(), StmtKind::ArrayAccessExpr);
	EXPECT_TRUE(Expr::classof(&arracc));
	EXPECT_TRUE(ArrayAccessExpr::classof(&arracc));

	// Function calls
	FunctionCallExpr callexpr;
	EXPECT_EQ(callexpr.getKind(), StmtKind::FunctionCallExpr);
	EXPECT_TRUE(Expr::classof(&callexpr));
	EXPECT_TRUE(FunctionCallExpr::classof(&callexpr));
}

TEST(ASTTests, StmtRTTI)
{
	// NullStmt
	NullStmt null;
	EXPECT_EQ(null.getKind(), StmtKind::NullStmt);
	EXPECT_FALSE(Expr::classof(&null));
	EXPECT_TRUE(NullStmt::classof(&null));

	// Return stmt
	ReturnStmt rtr;
	EXPECT_EQ(rtr.getKind(), StmtKind::ReturnStmt);
	EXPECT_FALSE(Expr::classof(&rtr));
	EXPECT_TRUE(ReturnStmt::classof(&rtr));

	// Condition
	ConditionStmt cond;
	EXPECT_EQ(cond.getKind(), StmtKind::ConditionStmt);
	EXPECT_FALSE(Expr::classof(&cond));
	EXPECT_TRUE(ConditionStmt::classof(&cond));

	// Compound
	CompoundStmt compound;
	EXPECT_EQ(compound.getKind(), StmtKind::CompoundStmt);
	EXPECT_FALSE(Expr::classof(&compound));
	EXPECT_TRUE(CompoundStmt::classof(&compound));

	// While
	WhileStmt whilestmt;
	EXPECT_EQ(whilestmt.getKind(), StmtKind::WhileStmt);
	EXPECT_FALSE(Expr::classof(&whilestmt));
	EXPECT_TRUE(WhileStmt::classof(&whilestmt));

	// declstmt
	DeclStmt declstmt(std::make_unique<FuncDecl>()); /* The arg passed to a DeclStmt cannot be null */
	EXPECT_EQ(declstmt.getKind(), StmtKind::DeclStmt);
	EXPECT_FALSE(Expr::classof(&declstmt));
	EXPECT_TRUE(DeclStmt::classof(&declstmt));
}

TEST(ASTTests, DeclRTTI)
{
	ASTContext astctxt;
	auto fooid = astctxt.identifiers.getUniqueIdentifierInfo("foo");
	auto intty = astctxt.getPrimitiveIntType();

	// Arg
	ParamDecl paramdecl;
	EXPECT_EQ(paramdecl.getKind(), DeclKind::ParamDecl);
	EXPECT_TRUE(ParamDecl::classof(&paramdecl));
	EXPECT_TRUE(NamedDecl::classof(&paramdecl));
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
	EXPECT_FALSE(DeclContext::classof(&vdecl));

	// Unit
	UnitDecl udecl(fooid,FileID());
	EXPECT_EQ(udecl.getKind(), DeclKind::UnitDecl);
	EXPECT_TRUE(UnitDecl::classof(&udecl));
	EXPECT_TRUE(NamedDecl::classof(&udecl));
	EXPECT_TRUE(DeclContext::classof(&udecl));
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