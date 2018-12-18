//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : ASTTests.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  (Unit) Tests for the AST nodes
//----------------------------------------------------------------------------//

#include "gtest/gtest.h"
#include "Support/TestUtils.hpp"
#include "Fox/AST/Decl.hpp"
#include "Fox/AST/Expr.hpp"
#include "Fox/AST/ASTContext.hpp"
#include "Fox/AST/Types.hpp"
#include "Fox/AST/ASTVisitor.hpp"
#include "Fox/Common/LLVM.hpp"
#include "Support/PrintObjects.hpp"

#include <algorithm>
#include <string>
#include <random>

using namespace fox;

// Tests that primitive types can be retrieve correctly
TEST(ASTTests, PrimitiveTypes) {
  ASTContext actxt;
  using PT = PrimitiveType;
  using PTK = PT::Kind;

  auto* primBool = PT::getBool(actxt);
  auto* primFloat  = PT::getFloat(actxt);
  auto* primInt  = PT::getInt(actxt);
  auto* primChar  = PT::getChar(actxt);
  auto* primString = PT::getString(actxt);
  auto* primVoid  = PT::getVoid(actxt);

  ASSERT_TRUE(primBool)  << "Ptr is null?";
  ASSERT_TRUE(primFloat)  << "Ptr is null?";
  ASSERT_TRUE(primInt)  << "Ptr is null?";
  ASSERT_TRUE(primChar)  << "Ptr is null?";
  ASSERT_TRUE(primString) << "Ptr is null?";
  ASSERT_TRUE(primVoid)  << "Ptr is null?";

  // Checks that they're all different
  EXPECT_NE(primBool, primFloat);
  EXPECT_NE(primFloat, primInt);
  EXPECT_NE(primInt, primChar);
  EXPECT_NE(primChar, primString);
  EXPECT_NE(primString, primVoid);

  // Test that the types have the correct properties
  // Bools
  EXPECT_EQ(primBool->getPrimitiveKind(), PTK::BoolTy);
  EXPECT_TRUE(primBool->isBoolType());

  // Floats
  EXPECT_EQ(primFloat->getPrimitiveKind(), PTK::FloatTy);
  EXPECT_TRUE(primFloat->isFloatType());

  // Ints
  EXPECT_EQ(primInt->getPrimitiveKind(), PTK::IntTy);
  EXPECT_TRUE(primInt->isIntType());

  // Chars
  EXPECT_EQ(primChar->getPrimitiveKind(), PTK::CharTy);
  EXPECT_TRUE(primChar->isCharType());

  // Strings
  EXPECT_EQ(primString->getPrimitiveKind(), PTK::StringTy);
  EXPECT_TRUE(primString->isStringType());

  // Void type
  EXPECT_EQ(primVoid->getPrimitiveKind(), PTK::VoidTy);
  EXPECT_TRUE(primVoid->isVoidType());

  // Check uniqueness
  EXPECT_EQ(primVoid, PT::getVoid(actxt));
  EXPECT_EQ(primInt, PT::getInt(actxt));
  EXPECT_EQ(primString, PT::getString(actxt));
  EXPECT_EQ(primChar, PT::getChar(actxt));
  EXPECT_EQ(primFloat, PT::getFloat(actxt));
  EXPECT_EQ(primBool, PT::getBool(actxt));
}

TEST(ASTTests, ASTContextArrayTypes) {
  ASTContext actxt;

  auto* primBool = PrimitiveType::getBool(actxt);
  auto* primFloat = PrimitiveType::getFloat(actxt);
  auto* primInt = PrimitiveType::getInt(actxt);
  auto* primChar = PrimitiveType::getChar(actxt);
  auto* primString = PrimitiveType::getString(actxt);

  auto* boolArr = ArrayType::get(actxt, primBool);
  auto* floatArr = ArrayType::get(actxt, primFloat);
  auto* intArr = ArrayType::get(actxt, primInt);
  auto* charArr = ArrayType::get(actxt, primChar);
  auto* strArr = ArrayType::get(actxt, primString);


  // Check that pointers aren't null
  ASSERT_TRUE(boolArr)  << "Pointer is null";
  ASSERT_TRUE(floatArr)  << "Pointer is null";
  ASSERT_TRUE(intArr)    << "Pointer is null";
  ASSERT_TRUE(charArr)  << "Pointer is null";
  ASSERT_TRUE(strArr)    << "Pointer is null";

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

  // Check that uniqueness works by getting the arraytype for int 
  EXPECT_EQ(ArrayType::get(actxt,primInt), intArr);
}

TEST(ASTTests, TypeRTTI) {
  ASTContext astctxt;
  TypeBase* intTy = PrimitiveType::getInt(astctxt);
  auto* arrIntTy = ArrayType::get(astctxt, intTy);
  auto* lvIntTy = LValueType::get(astctxt, intTy);
  auto* errType = ErrorType::get(astctxt);
  auto* cellType = CellType::create(astctxt);

  EXPECT_EQ(intTy->getKind(), TypeKind::PrimitiveType);
  EXPECT_TRUE(PrimitiveType::classof(intTy));
  EXPECT_TRUE(BasicType::classof(intTy));

  EXPECT_EQ(arrIntTy->getKind(), TypeKind::ArrayType);
  EXPECT_TRUE(ArrayType::classof(arrIntTy));

  EXPECT_EQ(lvIntTy->getKind(), TypeKind::LValueType);
  EXPECT_TRUE(LValueType::classof(lvIntTy));

  EXPECT_EQ(errType->getKind(), TypeKind::ErrorType);
  EXPECT_TRUE(ErrorType::classof(errType));
  EXPECT_TRUE(BasicType::classof(errType));

  EXPECT_EQ(cellType->getKind(), TypeKind::CellType);
  EXPECT_TRUE(CellType::classof(cellType));
}

TEST(ASTTests, ExprRTTI) {
  ASTContext ctxt;

  // Binary Exprs
  auto* binexpr = BinaryExpr::create(ctxt, BinaryExpr::OpKind::Invalid,
    nullptr, nullptr, SourceRange(), SourceRange());
  EXPECT_EQ(binexpr->getKind(), ExprKind::BinaryExpr);
  EXPECT_TRUE(BinaryExpr::classof(binexpr));

  // Unary Exprs
  auto* unaryexpr = UnaryExpr::create(ctxt, UnaryExpr::OpKind::Invalid, 
    nullptr, SourceRange(), SourceRange());
  EXPECT_EQ(unaryexpr->getKind(), ExprKind::UnaryExpr);
  EXPECT_TRUE(UnaryExpr::classof(unaryexpr));

  // Cast Exprs
  auto* castexpr = CastExpr::create(ctxt, TypeLoc(), nullptr, SourceRange());
  EXPECT_EQ(castexpr->getKind(), ExprKind::CastExpr);
  EXPECT_TRUE(CastExpr::classof(castexpr));

  // Literals
  auto* charlit = CharLiteralExpr::create(ctxt, '0', SourceRange());
  EXPECT_EQ(charlit->getKind(), ExprKind::CharLiteralExpr);
  EXPECT_TRUE(CharLiteralExpr::classof(charlit));

  auto* intlit = IntegerLiteralExpr::create(ctxt, 0, SourceRange());
  EXPECT_EQ(intlit->getKind(), ExprKind::IntegerLiteralExpr);
  EXPECT_TRUE(IntegerLiteralExpr::classof(intlit));

  auto* floatlit = FloatLiteralExpr::create(ctxt, 0.0, SourceRange());
  EXPECT_EQ(floatlit->getKind(), ExprKind::FloatLiteralExpr);
  EXPECT_TRUE(FloatLiteralExpr::classof(floatlit));

  auto* strlit = StringLiteralExpr::create(ctxt, "", SourceRange());
  EXPECT_EQ(strlit->getKind(), ExprKind::StringLiteralExpr);
  EXPECT_TRUE(StringLiteralExpr::classof(strlit));

  auto* boollit = BoolLiteralExpr::create(ctxt, false, SourceRange());
  EXPECT_EQ(boollit->getKind(), ExprKind::BoolLiteralExpr);
  EXPECT_TRUE(BoolLiteralExpr::classof(boollit));

  auto* arrlit = ArrayLiteralExpr::create(ctxt, (ExprVector()), SourceRange());
  EXPECT_EQ(arrlit->getKind(), ExprKind::ArrayLiteralExpr);
  EXPECT_TRUE(ArrayLiteralExpr::classof(arrlit));

  // Helper
  auto fooid = ctxt.getIdentifier("foo");

  auto* undeclref = UnresolvedDeclRefExpr::create(ctxt,
    (Identifier()), SourceRange());
  EXPECT_EQ(undeclref->getKind(), ExprKind::UnresolvedDeclRefExpr);
  EXPECT_TRUE(UnresolvedDeclRefExpr::classof(undeclref));

  // DeclRef
  auto* declref = DeclRefExpr::create(ctxt, nullptr, SourceRange());
  EXPECT_EQ(declref->getKind(), ExprKind::DeclRefExpr);
  EXPECT_TRUE(DeclRefExpr::classof(declref));

  // MemberOfExpr
  auto* membof = MemberOfExpr::create(ctxt, nullptr, Identifier(),
    SourceRange(), SourceLoc());
  EXPECT_EQ(membof->getKind(), ExprKind::MemberOfExpr);
  EXPECT_TRUE(MemberOfExpr::classof(membof));

  // Array Access
  auto* arracc = ArraySubscriptExpr::create(ctxt, nullptr, nullptr, 
    SourceRange());
  EXPECT_EQ(arracc->getKind(), ExprKind::ArraySubscriptExpr);
  EXPECT_TRUE(ArraySubscriptExpr::classof(arracc));

  // Function calls
  auto* callexpr = FunctionCallExpr::create(ctxt, nullptr,
    ExprVector(), SourceRange());
  EXPECT_EQ(callexpr->getKind(), ExprKind::FunctionCallExpr);
  EXPECT_TRUE(FunctionCallExpr::classof(callexpr));
}

TEST(ASTTests, StmtRTTI) {
  ASTContext ctxt;

  // NullStmt
  auto* null = NullStmt::create(ctxt);
  EXPECT_EQ(null->getKind(), StmtKind::NullStmt);
  EXPECT_TRUE(NullStmt::classof(null));

  // Return stmt
  auto* rtr = ReturnStmt::create(ctxt, nullptr, SourceRange());
  EXPECT_EQ(rtr->getKind(), StmtKind::ReturnStmt);
  EXPECT_TRUE(ReturnStmt::classof(rtr));

  // Condition
  auto* cond = ConditionStmt::create(ctxt, nullptr, ASTNode(), ASTNode(),
    SourceRange(), SourceLoc());
  EXPECT_EQ(cond->getKind(), StmtKind::ConditionStmt);
  EXPECT_TRUE(ConditionStmt::classof(cond));

  // Compound
  auto* compound = CompoundStmt::create(ctxt, SourceRange());
  EXPECT_EQ(compound->getKind(), StmtKind::CompoundStmt);
  EXPECT_TRUE(CompoundStmt::classof(compound));

  // While
  auto* whilestmt = 
    WhileStmt::create(ctxt, nullptr, ASTNode(), SourceRange(), SourceLoc());
  EXPECT_EQ(whilestmt->getKind(), StmtKind::WhileStmt);
  EXPECT_TRUE(WhileStmt::classof(whilestmt));
}

namespace {
  VarDecl* createEmptyVarDecl(ASTContext& ctxt, DeclContext* dc = nullptr) {
    return VarDecl::create(ctxt, dc, Identifier(), SourceRange(), TypeLoc(), 
      false, nullptr, SourceRange());
  }

  FuncDecl* createEmptyFnDecl(ASTContext& ctxt, DeclContext* dc = nullptr) {
    return FuncDecl::create(ctxt, dc, Identifier(), SourceRange(), TypeLoc(), 
      SourceRange(), SourceLoc());
  }

  ParamDecl* createEmptyParamDecl(ASTContext& ctxt) {
    return ParamDecl::create(ctxt, nullptr, Identifier(), SourceRange(),
      TypeLoc(), false, SourceRange());
  }
}

TEST(ASTTests, DeclRTTI) {
  ASTContext ctxt;
  // Arg
  ParamDecl* paramdecl = createEmptyParamDecl(ctxt);
  EXPECT_EQ(paramdecl->getKind(), DeclKind::ParamDecl);
  EXPECT_TRUE(ParamDecl::classof(paramdecl));
  EXPECT_TRUE(NamedDecl::classof(paramdecl));
  EXPECT_TRUE(ValueDecl::classof(paramdecl));
  EXPECT_FALSE(DeclContext::classof(paramdecl));

  // Func
  auto* fndecl = createEmptyFnDecl(ctxt);
  EXPECT_EQ(fndecl->getKind(), DeclKind::FuncDecl);
  EXPECT_TRUE(FuncDecl::classof((Decl*)fndecl));
  EXPECT_TRUE(NamedDecl::classof(fndecl));

  // Var
  auto* vdecl = createEmptyVarDecl(ctxt);
  EXPECT_EQ(vdecl->getKind(), DeclKind::VarDecl);
  EXPECT_TRUE(VarDecl::classof(vdecl));
  EXPECT_TRUE(NamedDecl::classof(vdecl));
  EXPECT_TRUE(ValueDecl::classof(vdecl));
  EXPECT_FALSE(DeclContext::classof(vdecl));

  // Unit
  Identifier id; FileID fid;
  DeclContext dc(DeclContextKind::UnitDecl);
  UnitDecl* udecl = UnitDecl::create(ctxt, &dc, id, fid);
  EXPECT_EQ(udecl->getKind(), DeclKind::UnitDecl);
  EXPECT_EQ(udecl->getDeclContextKind(), DeclContextKind::UnitDecl);
  EXPECT_TRUE(UnitDecl::classof((Decl*)udecl));
  EXPECT_TRUE(DeclContext::classof(udecl));
}

TEST(ASTTests, DeclDeclContextRTTI) {
  ASTContext ctxt;
  Identifier id; FileID fid;
  DeclContext dc(DeclContextKind::UnitDecl);

  // UnitDecl -> DeclContext -> UnitDecl
  UnitDecl* udecl = UnitDecl::create(ctxt, &dc, id, fid);
  DeclContext* tmp = udecl;
  EXPECT_EQ(udecl, dyn_cast<UnitDecl>(tmp));
}

// ASTVisitor tests : Samples implementations to test if visitors works as intended
class IsNamedDecl : public SimpleASTVisitor<IsNamedDecl, bool> {
  public:
    bool visitNamedDecl(NamedDecl* node) {
      return true;
    }
};

class IsExpr : public SimpleASTVisitor<IsExpr, bool> {
  public:
    bool visitExpr(Expr* node) {
      return true;
    }
};

class IsArrTy : public SimpleASTVisitor<IsArrTy, bool> {
  public:
    bool visitArrayType(ArrayType* node) {
      return true;
    }
};

TEST(ASTTests, BasicVisitor) {
  // Context
  ASTContext ctxt;

  // Create test nodes
  auto* intlit = IntegerLiteralExpr::create(ctxt, 42, SourceRange());
  auto* rtr = ReturnStmt::create(ctxt, nullptr, SourceRange());
  auto* vardecl = createEmptyVarDecl(ctxt);
  auto* intTy = PrimitiveType::getInt(ctxt);
  auto* arrInt = ArrayType::get(ctxt, intTy);

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

// Number of identifiers to insert into the table in the 
// "randomIdentifierInsertion" test.
#define RANDOM_ID_TEST_NUMBER_OF_ID 2048
// Note, if theses values are too low, the test might fail 
// sometimes because there's a change that the randomly generated
// identifier is already taken. Using high values 
// make the test longer, but also a lot less unlikely to fail!
#define RANDOM_STRING_MIN_LENGTH 128
#define RANDOM_STRING_MAX_LENGTH 128

namespace {
	const std::string 
	idStrChars = "_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

	std::string generateRandomString() {
		std::random_device rd;
		std::mt19937_64 mt(rd());
		std::uniform_int_distribution<int> dist_char(0, idStrChars.size()-1);

		std::uniform_int_distribution<int> dist_length(RANDOM_STRING_MIN_LENGTH, RANDOM_STRING_MAX_LENGTH);
		int strlen = dist_length(mt);

		std::string output;
		std::generate_n(std::back_inserter(output), strlen, [&] {return idStrChars[dist_char(mt)]; });
		return output;
	}
}

// Checks if Identifiers are unique, or not.
TEST(IdentifierTableTests, identifiersUniqueness) {
  // Create 2 identifiers, A and B
  std::string rawIdA, rawIdB;
  rawIdA = generateRandomString();
  rawIdB = generateRandomString();
  ASSERT_NE(rawIdA, rawIdB) << "Generated 2 equal random identifiers";

  ASTContext ctxt;
  Identifier idA = ctxt.getIdentifier(rawIdA);
  Identifier idB = ctxt.getIdentifier(rawIdB);

  ASSERT_NE(idA, idB);
  ASSERT_NE(idA.getStr(), idB.getStr());
}

// Checks if the ASTContext supports large identifiers 
// amount by inserting a lot of random ids.
TEST(ASTTests, randomIdentifierInsertion) {
	ASTContext ctxt;
  Identifier lastId;
  std::vector<Identifier> allIdentifiers;
  std::vector<std::string> allIdStrs;
	std::string id;
  for (std::size_t k(0); k < RANDOM_ID_TEST_NUMBER_OF_ID; k++) {
    id = generateRandomString();
        
    auto idinfo = ctxt.getIdentifier(id);
    // Check if the string matches, and if the adress of this type
		// is different from the last one used.
    ASSERT_EQ(idinfo.getStr().to_string(), id) << "[Insertion " << k 
			<< "] Strings did not match";
    ASSERT_NE(lastId, idinfo) 
			<< "[Insertion " << k 
			<< "] Insertion returned the same Identifier object for 2 different strings";
    
    lastId = idinfo;
    
    allIdStrs.push_back(id);
    allIdentifiers.push_back(idinfo);
  }

  // Now, iterate over all identifierinfo to check if they're still valid 
  for (std::size_t idx(0);idx < allIdentifiers.size(); idx++) {
    ASSERT_FALSE(allIdentifiers[idx].isNull()) << "Pointer can't be null";
    ASSERT_EQ(allIdStrs[idx], allIdentifiers[idx].getStr());
  }
}