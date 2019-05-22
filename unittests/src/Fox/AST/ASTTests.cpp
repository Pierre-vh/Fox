//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : ASTTests.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  (Unit) Tests for the AST nodes
//----------------------------------------------------------------------------//

#include "gtest/gtest.h"
#include "Fox/AST/ASTContext.hpp"
#include "Fox/AST/Decl.hpp"
#include "Fox/AST/Expr.hpp"
#include "Fox/AST/Types.hpp"
#include "Fox/AST/ASTVisitor.hpp"
#include "Fox/AST/TypeVisitor.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"
#include "Fox/Common/SourceManager.hpp"
#include "Support/TestUtils.hpp"
#include <algorithm>
#include <string>
#include <random>
#include <iostream>

using namespace fox;

namespace {
  class ASTTest : public testing::Test {
    public:
      ASTTest() : diags(srcMgr, std::cout), ctxt(srcMgr, diags) {}

    protected:
      SourceManager srcMgr;
      DiagnosticEngine diags;
      ASTContext ctxt;
  };
}

TEST_F(ASTTest, BasicType) {
  auto primBool     = BoolType::get(ctxt);
  auto primDouble   = DoubleType::get(ctxt);
  auto primInt      = IntegerType::get(ctxt);
  auto primChar     = CharType::get(ctxt);
  auto primString   = StringType::get(ctxt);
  auto primVoid     = VoidType::get(ctxt);
  auto errType      = ErrorType::get(ctxt);

  // Check that they're not null
  ASSERT_NE(primBool,   nullptr);
  ASSERT_NE(primDouble, nullptr);
  ASSERT_NE(primInt,    nullptr);
  ASSERT_NE(primChar,   nullptr);
  ASSERT_NE(primString, nullptr);
  ASSERT_NE(primVoid,   nullptr);
  ASSERT_NE(errType,   nullptr);

  // Test that the types have the correct properties
  EXPECT_TRUE(primBool->isBoolType());
  EXPECT_TRUE(primDouble->isDoubleType());
  EXPECT_TRUE(primInt->isIntType());
  EXPECT_TRUE(primChar->isCharType());
  EXPECT_TRUE(primString->isStringType());
  EXPECT_TRUE(primVoid->isVoidType());
  EXPECT_TRUE(errType->hasErrorType());

  // Check uniqueness
  EXPECT_EQ(primVoid,   VoidType::get(ctxt));
  EXPECT_EQ(primInt,    IntegerType::get(ctxt));
  EXPECT_EQ(primString, StringType::get(ctxt));
  EXPECT_EQ(primChar,   CharType::get(ctxt));
  EXPECT_EQ(primDouble, DoubleType::get(ctxt));
  EXPECT_EQ(primBool,   BoolType::get(ctxt));
  EXPECT_EQ(errType,    ErrorType::get(ctxt));
}

TEST_F(ASTTest, ASTContextArrayTypes) {
  Type primBool     = BoolType::get(ctxt);
  Type primDouble   = DoubleType::get(ctxt);
  Type primInt      = IntegerType::get(ctxt);
  Type primChar     = CharType::get(ctxt);
  Type primString   = StringType::get(ctxt);

  ArrayType* boolArr  = ArrayType::get(ctxt, primBool);
  ArrayType* dblArr   = ArrayType::get(ctxt, primDouble);
  ArrayType* intArr   = ArrayType::get(ctxt, primInt);
  ArrayType* charArr  = ArrayType::get(ctxt, primChar);
  ArrayType* strArr   = ArrayType::get(ctxt, primString);

  // Check that pointers aren't null
  ASSERT_NE(boolArr,  nullptr);
  ASSERT_NE(dblArr,   nullptr);
  ASSERT_NE(intArr,   nullptr);
  ASSERT_NE(charArr,  nullptr);
  ASSERT_NE(strArr,   nullptr);

  // Check that itemTypes are correct
  EXPECT_EQ(boolArr->getElementType(),  primBool);
  EXPECT_EQ(dblArr->getElementType(),   primDouble);
  EXPECT_EQ(intArr->getElementType(),   primInt);
  EXPECT_EQ(charArr->getElementType(),  primChar);
  EXPECT_EQ(strArr->getElementType(),   primString);

  // Check that uniqueness works
  EXPECT_EQ(ArrayType::get(ctxt, primInt), intArr);
  // TODO: Add more?
}

TEST_F(ASTTest, TypeRTTI) {
  TypeBase* intType    = IntegerType::get(ctxt);
  EXPECT_EQ(intType->getKind(), TypeKind::IntegerType);
  EXPECT_TRUE(IntegerType::classof(intType));
  EXPECT_TRUE(BasicType::classof(intType));
  EXPECT_TRUE(PrimitiveType::classof(intType));

  TypeBase* doubleType = DoubleType::get(ctxt);
  EXPECT_EQ(doubleType->getKind(), TypeKind::DoubleType);
  EXPECT_TRUE(DoubleType::classof(doubleType));
  EXPECT_TRUE(BasicType::classof(doubleType));
  EXPECT_TRUE(PrimitiveType::classof(doubleType));

  TypeBase* boolType   = BoolType::get(ctxt);
  EXPECT_EQ(boolType->getKind(), TypeKind::BoolType);
  EXPECT_TRUE(BoolType::classof(boolType));
  EXPECT_TRUE(BasicType::classof(boolType));
  EXPECT_TRUE(PrimitiveType::classof(boolType));

  TypeBase* charType   = CharType::get(ctxt);
  EXPECT_EQ(charType->getKind(), TypeKind::CharType);
  EXPECT_TRUE(CharType::classof(charType));
  EXPECT_TRUE(BasicType::classof(charType));
  EXPECT_TRUE(PrimitiveType::classof(charType));

  TypeBase* strType    = StringType::get(ctxt);
  EXPECT_EQ(strType->getKind(), TypeKind::StringType);
  EXPECT_TRUE(StringType::classof(strType));
  EXPECT_TRUE(BasicType::classof(strType));
  EXPECT_TRUE(PrimitiveType::classof(strType));

  TypeBase* voidType   = VoidType::get(ctxt);
  EXPECT_EQ(voidType->getKind(), TypeKind::VoidType);
  EXPECT_TRUE(VoidType::classof(voidType));
  EXPECT_TRUE(BasicType::classof(voidType));
  EXPECT_TRUE(PrimitiveType::classof(voidType));

  TypeBase* arrIntTy = ArrayType::get(ctxt, intType);
  EXPECT_EQ(arrIntTy->getKind(), TypeKind::ArrayType);
  EXPECT_TRUE(ArrayType::classof(arrIntTy));

  TypeBase* lvIntTy = LValueType::get(ctxt, intType);
  EXPECT_EQ(lvIntTy->getKind(), TypeKind::LValueType);
  EXPECT_TRUE(LValueType::classof(lvIntTy));

  TypeBase* errType = ErrorType::get(ctxt);
  EXPECT_EQ(errType->getKind(), TypeKind::ErrorType);
  EXPECT_TRUE(ErrorType::classof(errType));
  EXPECT_TRUE(BasicType::classof(errType));

  TypeBase* funcType = FunctionType::get(ctxt, {intType}, intType);
  EXPECT_EQ(funcType->getKind(), TypeKind::FunctionType);
  EXPECT_TRUE(FunctionType::classof(funcType));

  TypeBase* tyVarType = TypeVariableType::create(ctxt, 0);
  EXPECT_EQ(tyVarType->getKind(), TypeKind::TypeVariableType);
  EXPECT_TRUE(TypeVariableType::classof(tyVarType));
}

TEST_F(ASTTest, ExprRTTI) {
  {
    auto* expr = BinaryExpr::create(ctxt, BinaryExpr::OpKind::Invalid,
      nullptr, nullptr, SourceRange());
    EXPECT_EQ(expr->getKind(), ExprKind::BinaryExpr);
    EXPECT_TRUE(BinaryExpr::classof(expr));
  }

  {
    auto* expr = UnaryExpr::create(ctxt, UnaryExpr::OpKind::Invalid, 
      nullptr, SourceRange());
    EXPECT_EQ(expr->getKind(), ExprKind::UnaryExpr);
    EXPECT_TRUE(UnaryExpr::classof(expr));
  }

  {
    auto* expr = CastExpr::create(ctxt, TypeLoc(), nullptr);
    EXPECT_EQ(expr->getKind(), ExprKind::CastExpr);
    EXPECT_TRUE(CastExpr::classof(expr));
  }

  {
    auto* expr = CharLiteralExpr::create(ctxt, '0', SourceRange());
    EXPECT_EQ(expr->getKind(), ExprKind::CharLiteralExpr);
    EXPECT_TRUE(CharLiteralExpr::classof(expr));
  }

  {
    auto* expr = IntegerLiteralExpr::create(ctxt, 0, SourceRange());
    EXPECT_EQ(expr->getKind(), ExprKind::IntegerLiteralExpr);
    EXPECT_TRUE(IntegerLiteralExpr::classof(expr));
  }

  {
    auto* expr = DoubleLiteralExpr::create(ctxt, 0.0, SourceRange());
    EXPECT_EQ(expr->getKind(), ExprKind::DoubleLiteralExpr);
    EXPECT_TRUE(DoubleLiteralExpr::classof(expr));
  }

  {
    auto* expr = StringLiteralExpr::create(ctxt, string_view(), SourceRange());
    EXPECT_EQ(expr->getKind(), ExprKind::StringLiteralExpr);
    EXPECT_TRUE(StringLiteralExpr::classof(expr));
  }

  {
    auto* expr = BoolLiteralExpr::create(ctxt, false, SourceRange());
    EXPECT_EQ(expr->getKind(), ExprKind::BoolLiteralExpr);
    EXPECT_TRUE(BoolLiteralExpr::classof(expr));
  }

  {
    auto* expr = ArrayLiteralExpr::create(ctxt, (ExprVector()), SourceRange());
    EXPECT_EQ(expr->getKind(), ExprKind::ArrayLiteralExpr);
    EXPECT_TRUE(ArrayLiteralExpr::classof(expr));
  }

  {
    auto* expr = UnresolvedDeclRefExpr::create(ctxt,
      (Identifier()), SourceRange());
    EXPECT_EQ(expr->getKind(), ExprKind::UnresolvedDeclRefExpr);
    EXPECT_TRUE(UnresolvedDeclRefExpr::classof(expr));
  }

  {
    auto* expr = DeclRefExpr::create(ctxt, nullptr, SourceRange());
    EXPECT_EQ(expr->getKind(), ExprKind::DeclRefExpr);
    EXPECT_TRUE(DeclRefExpr::classof(expr));
  }

  {
    auto* expr = UnresolvedDotExpr::create(ctxt, nullptr, Identifier(),
      SourceRange(), SourceLoc());
    EXPECT_EQ(expr->getKind(), ExprKind::UnresolvedDotExpr);
    EXPECT_TRUE(UnresolvedDotExpr::classof(expr));
  }

  {
    auto* expr = BuiltinMemberRefExpr::create(ctxt, nullptr, Identifier(),
      SourceRange(), SourceLoc(), BuiltinTypeMemberKind(-1));
    EXPECT_EQ(expr->getKind(), ExprKind::BuiltinMemberRefExpr);
    EXPECT_TRUE(BuiltinMemberRefExpr::classof(expr));
  }

  {
    auto* expr = SubscriptExpr::create(ctxt, nullptr, nullptr, SourceLoc());
    EXPECT_EQ(expr->getKind(), ExprKind::SubscriptExpr);
    EXPECT_TRUE(SubscriptExpr::classof(expr));
  }

  {
    auto* expr = CallExpr::create(ctxt, nullptr, ExprVector(), SourceLoc());
    EXPECT_EQ(expr->getKind(), ExprKind::CallExpr);
    EXPECT_TRUE(CallExpr::classof(expr));
  }

  {
    auto* expr = ErrorExpr::create(ctxt, SourceRange());
    EXPECT_EQ(expr->getKind(), ExprKind::ErrorExpr);
    EXPECT_TRUE(ErrorExpr::classof(expr));
  }
}

TEST_F(ASTTest, ExprFlags) {
  {
    CastExpr* cast = CastExpr::create(ctxt, TypeLoc(), nullptr);
    // CastExprs shouldn't be useless by default
    EXPECT_FALSE(cast->isUseless());
    cast->markAsUselesss();
    EXPECT_TRUE(cast->isUseless());
  }
  // TODO: Add more of theses
}

TEST_F(ASTTest, StmtRTTI) {
  // Return stmt
  auto* rtr = ReturnStmt::create(ctxt, nullptr, SourceRange());
  EXPECT_EQ(rtr->getKind(), StmtKind::ReturnStmt);
  EXPECT_TRUE(ReturnStmt::classof(rtr));

  // Condition
  auto* cond = ConditionStmt::create(ctxt, SourceLoc(), 
                                     nullptr, nullptr, nullptr);
  EXPECT_EQ(cond->getKind(), StmtKind::ConditionStmt);
  EXPECT_TRUE(ConditionStmt::classof(cond));

  // Compound
  auto* compound = CompoundStmt::create(ctxt, ArrayRef<ASTNode>(), 
    SourceRange());
  EXPECT_EQ(compound->getKind(), StmtKind::CompoundStmt);
  EXPECT_TRUE(CompoundStmt::classof(compound));

  // While
  auto* whilestmt = 
    WhileStmt::create(ctxt, SourceLoc(), nullptr, nullptr);
  EXPECT_EQ(whilestmt->getKind(), StmtKind::WhileStmt);
  EXPECT_TRUE(WhileStmt::classof(whilestmt));
}

namespace {
  VarDecl* createEmptyVarDecl(ASTContext& ctxt, DeclContext* dc) {
    return VarDecl::create(ctxt, dc, VarDecl::Keyword::Let, SourceRange(),
      Identifier(), SourceRange(), TypeLoc(), nullptr);
  }

  FuncDecl* createEmptyFnDecl(ASTContext& ctxt, UnitDecl* unit) {
    return 
      FuncDecl::create(ctxt, unit, SourceLoc(), Identifier(), SourceRange());
  }

  ParamDecl* createEmptyParamDecl(ASTContext& ctxt, FuncDecl* func) {
    return ParamDecl::create(ctxt, func, Identifier(), SourceRange(),
      TypeLoc(), false);
  }
}

TEST_F(ASTTest, DeclRTTI) {
  // Unit
  Identifier id; FileID fid;
  UnitDecl* udecl = UnitDecl::create(ctxt, id, fid);
  EXPECT_EQ(udecl->getKind(), DeclKind::UnitDecl);
  EXPECT_EQ(udecl->getDeclContextKind(), DeclContextKind::UnitDecl);
  EXPECT_TRUE(UnitDecl::classof((Decl*)udecl));
  EXPECT_TRUE(DeclContext::classof(udecl));

  // Func
  auto* fndecl = createEmptyFnDecl(ctxt, udecl);
  EXPECT_EQ(fndecl->getKind(), DeclKind::FuncDecl);
  EXPECT_TRUE(FuncDecl::classof((Decl*)fndecl));
  EXPECT_TRUE(NamedDecl::classof(fndecl));

  // BuiltinFunc
  auto* builtin = BuiltinFuncDecl::get(ctxt, BuiltinID::printBool);
  EXPECT_EQ(builtin->getKind(), DeclKind::BuiltinFuncDecl);
  EXPECT_TRUE(BuiltinFuncDecl::classof((Decl*)builtin));
  EXPECT_TRUE(NamedDecl::classof(builtin));

  // Arg
  ParamDecl* paramdecl = createEmptyParamDecl(ctxt, fndecl);
  EXPECT_EQ(paramdecl->getKind(), DeclKind::ParamDecl);
  EXPECT_TRUE(ParamDecl::classof(paramdecl));
  EXPECT_TRUE(NamedDecl::classof(paramdecl));
  EXPECT_TRUE(ValueDecl::classof(paramdecl));
  EXPECT_FALSE(DeclContext::classof(paramdecl));

  // Var
  auto* vdecl = createEmptyVarDecl(ctxt, fndecl);
  EXPECT_EQ(vdecl->getKind(), DeclKind::VarDecl);
  EXPECT_TRUE(VarDecl::classof(vdecl));
  EXPECT_TRUE(NamedDecl::classof(vdecl));
  EXPECT_TRUE(ValueDecl::classof(vdecl));
  EXPECT_FALSE(DeclContext::classof(vdecl));
}

TEST_F(ASTTest, DeclDeclContextRTTI) {
  Identifier id; FileID fid;
  // UnitDecl -> DeclContext -> UnitDecl
  UnitDecl* udecl = UnitDecl::create(ctxt, id, fid);
  DeclContext* tmp = udecl;
  EXPECT_EQ(udecl, dyn_cast<UnitDecl>(tmp));
}

namespace {
  // Sample AST/TypeVisitors to test that they work as intended.

  class IsNamedDecl : public SimpleASTVisitor<IsNamedDecl, bool> {
    public:
      bool visitStmt(Stmt*) { return false; };
      bool visitDecl(Decl*) { return false; };
      bool visitExpr(Expr*) { return false; }
      bool visitNamedDecl(NamedDecl*) {
        return true;
      }
  };

  class IsExpr : public SimpleASTVisitor<IsExpr, bool> {
    public:
      bool visitStmt(Stmt*) { return false; };
      bool visitDecl(Decl*) { return false; };
      bool visitExpr(Expr*) { return true; }
  };

  class IsArrTy : public TypeVisitor<IsArrTy, bool> {
    public:
      bool visitTypeBase(TypeBase*) { return false; }
      bool visitArrayType(ArrayType*) { return true; }
  };
}

TEST_F(ASTTest, BasicVisitor) {
  // Create test nodes
  auto* intlit = IntegerLiteralExpr::create(ctxt, 42, SourceRange());
  auto* rtr = ReturnStmt::create(ctxt, nullptr, SourceRange());
  UnitDecl* unit = UnitDecl::create(ctxt, Identifier(), FileID());
  auto* vardecl = createEmptyVarDecl(ctxt, unit);
  Type intTy = IntegerType::get(ctxt);
  Type arrInt = ArrayType::get(ctxt, intTy);

  IsExpr exprVisitor;
  IsNamedDecl declVisitor;
  IsArrTy tyVisitor;

  EXPECT_TRUE(exprVisitor.visit(intlit));
  EXPECT_FALSE(exprVisitor.visit(rtr));
  EXPECT_FALSE(exprVisitor.visit(vardecl));

  EXPECT_FALSE(declVisitor.visit(intlit));
  EXPECT_FALSE(declVisitor.visit(rtr));
  EXPECT_TRUE(declVisitor.visit(vardecl));

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
TEST_F(ASTTest, identifiersUniqueness) {
  // Create 2 identifiers, A and B
  std::string rawIdA, rawIdB;
  rawIdA = generateRandomString();
  rawIdB = generateRandomString();
  ASSERT_NE(rawIdA, rawIdB) << "Generated 2 equal random identifiers";

  Identifier idA = ctxt.getIdentifier(rawIdA);
  Identifier idB = ctxt.getIdentifier(rawIdB);
  // Check that the pointers are different
  ASSERT_NE(idA, idB);
  ASSERT_NE(idA.getStr(), idB.getStr());
  // Check that comparison succeeds
  for (unsigned k = 0; k < 10; ++k) {
    Identifier newIDA = ctxt.getIdentifier(rawIdA);
    EXPECT_EQ(idA.c_str(), newIDA.c_str());
    EXPECT_EQ(idA, newIDA);
    Identifier newIDB = ctxt.getIdentifier(rawIdB);
    EXPECT_EQ(idB.c_str(), newIDB.c_str());
    EXPECT_EQ(idB, newIDB);
  }
}

// Checks if the ASTContext supports large identifiers 
// amount by inserting a lot of random ids.
TEST_F(ASTTest, randomIdentifierInsertion) {
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

TEST_F(ASTTest, allocateCopyOfString) {
  constexpr auto theLiteral = "This is a bad practice!";
  string_view cpy;
  const char* danglingTempPtr = nullptr;
  {
    std::string temp = theLiteral;
    danglingTempPtr = temp.data();
    cpy = ctxt.allocateCopy(temp);
  }
  // Check that memory was allocated in a different region
  ASSERT_NE(cpy.data(), danglingTempPtr);
  // Check that the memory can be accessed safely
  EXPECT_EQ(cpy, theLiteral);
}

TEST_F(ASTTest, cleanup) {
  int callCount = 0;
  auto callMe = [&](){
    callCount++;
  };
  ctxt.addCleanup(callMe);
  ctxt.addCleanup(callMe);
  ctxt.addCleanup(callMe);
  ctxt.addCleanup(callMe);
  ctxt.addCleanup(callMe);
  EXPECT_EQ(callCount, 0);
  ctxt.reset();
  EXPECT_EQ(callCount, 5);
}

TEST_F(ASTTest, functionTypesUniqueness) {
  Type intTy = IntegerType::get(ctxt);
  Type boolTy = BoolType::get(ctxt);
  Type voidTy = VoidType::get(ctxt);

  Type fns[6];
  // Create a few functions with different signatures.
  // (int, bool) -> void
  fns[0] = FunctionType::get(ctxt, {intTy, boolTy}, voidTy);
  // (int) -> bool
  fns[1] = FunctionType::get(ctxt, {intTy}, boolTy);
  // (bool) -> int
  fns[2] = FunctionType::get(ctxt, {boolTy}, intTy);
  // () -> void
  fns[3] = FunctionType::get(ctxt, {}, voidTy);
  // (bool, int) -> void
  fns[4] = FunctionType::get(ctxt, {boolTy, intTy}, voidTy);
  // (bool) -> void
  fns[5] = FunctionType::get(ctxt, {boolTy}, voidTy);

  // Check that they all have different pointers
  std::set<TypeBase*> ptrs;
  for(std::size_t k = 0; k < 6; k++) {
    auto result = ptrs.insert(fns[k].getPtr());
    EXPECT_TRUE(result.second) << "element already exists";
  }

  // Check that we can successfully retrieve every function type singleton
  EXPECT_EQ(fns[0], FunctionType::get(ctxt, {intTy, boolTy}, voidTy));
  EXPECT_EQ(fns[1], FunctionType::get(ctxt, {intTy}, boolTy));
  EXPECT_EQ(fns[2], FunctionType::get(ctxt, {boolTy}, intTy));
  EXPECT_EQ(fns[3], FunctionType::get(ctxt, {}, voidTy));
  EXPECT_EQ(fns[4], FunctionType::get(ctxt, {boolTy, intTy}, voidTy));
  EXPECT_EQ(fns[5], FunctionType::get(ctxt, {boolTy}, voidTy));
}

class ASTBuiltinsTest : public ::testing::Test {
  public:
    ASTBuiltinsTest() : diags(srcMgr, std::cout), ctxt(srcMgr, diags) {}

    SourceManager srcMgr;
    DiagnosticEngine diags;
    ASTContext ctxt;

};

TEST_F(ASTBuiltinsTest, builtinFuncTypes) {
  Type printIntTy = ctxt.getBuiltinFuncType(BuiltinID::printInt);
  Type printBoolTy = ctxt.getBuiltinFuncType(BuiltinID::printBool);
  Type printStrTy = ctxt.getBuiltinFuncType(BuiltinID::printString);
  Type strConcatTy = ctxt.getBuiltinFuncType(BuiltinID::strConcat);

  Type printIntRtrTy = ctxt.getBuiltinFuncReturnType(BuiltinID::printInt);
  Type printBoolRtrTy = ctxt.getBuiltinFuncReturnType(BuiltinID::printBool);
  Type printStrRtrTy = ctxt.getBuiltinFuncReturnType(BuiltinID::printString);
  Type strConcatRtrTy = ctxt.getBuiltinFuncReturnType(BuiltinID::strConcat);

  // FIXME: Is 'toDebugString' fine for this?
  EXPECT_EQ(printIntTy->toDebugString(),  "(int) -> void");
  EXPECT_EQ(printBoolTy->toDebugString(), "(bool) -> void");
  EXPECT_EQ(printStrTy->toDebugString(),  "(string) -> void");
  EXPECT_EQ(strConcatTy->toDebugString(), "(string, string) -> string");

  EXPECT_TRUE(printIntRtrTy->isVoidType());
  EXPECT_TRUE(printBoolRtrTy->isVoidType());
  EXPECT_TRUE(printStrRtrTy->isVoidType());
  EXPECT_TRUE(strConcatRtrTy->isStringType());
}

TEST_F(ASTBuiltinsTest, builtinIdentifier) {
  EXPECT_EQ(ctxt.getIdentifier(BuiltinID::printBool).getStr(), 
            "printBool");
  EXPECT_EQ(ctxt.getIdentifier(BuiltinID::printBool),
            ctxt.getIdentifier(BuiltinID::printBool));

  EXPECT_EQ(ctxt.getIdentifier(BuiltinID::printInt).getStr(), 
            "printInt");
  EXPECT_EQ(ctxt.getIdentifier(BuiltinID::printInt),
            ctxt.getIdentifier(BuiltinID::printInt));
}

TEST_F(ASTBuiltinsTest, builtinLookup) {
  auto printBool = ctxt.getIdentifier(BuiltinID::printBool);
  auto printInt = ctxt.getIdentifier(BuiltinID::printInt);
  {
    SmallVector<BuiltinFuncDecl*, 4> results;
    ctxt.lookupBuiltin(printBool, results);
    ASSERT_EQ(results.size(), 1u) 
      << "Incorrect number of results for " << printBool.getStr();
    EXPECT_EQ(results.front(), BuiltinFuncDecl::get(ctxt, BuiltinID::printBool));
  }
  {
    SmallVector<BuiltinFuncDecl*, 4> results;
    ctxt.lookupBuiltin(printInt, results);
    ASSERT_EQ(results.size(), 1u) 
      << "Incorrect number of results for " << printInt.getStr();
    EXPECT_EQ(results.front(), BuiltinFuncDecl::get(ctxt, BuiltinID::printInt));
  }
}

namespace {
  std::string 
  dumpAmbiguousBuiltinErr(
    const SmallVectorImpl<std::pair<string_view, Type>>& pairs
  ) {
    std::stringstream ss;
    ss << "{\n";
    for (auto pair : pairs)
      ss << '\'' << pair.first << "' -> '"
         << pair.second->toDebugString() << "'\n";
    ss << "}\n";
    return ss.str();
  }
}

TEST_F(ASTBuiltinsTest, unambiguousBuiltins) {
  // Tests that builtin functions identifiers are not ambiguous
  std::unordered_map<
    string_view, 
    SmallVector<std::pair<string_view, Type>, 2>
  > buitlins;
  #define PUBLIC_BUILTIN(FUNC, FOX)\
    buitlins[#FOX].push_back({#FUNC, ctxt.getBuiltinFuncType(BuiltinID::FUNC)});
  #include "Fox/Common/Builtins.def"
  // For now, simply check that every vector in the array has a size of one.
  for (auto builtin : buitlins) {
    ASSERT_GT(builtin.second.size(), 0u) 
      << "Builtin '" << builtin.first 
      << "' doesn't have a function associated with it?";
    EXPECT_EQ(builtin.second.size(), 1u)
      << "Builtins with identifier '" << builtin.first 
      << "' are ambiguous: " << dumpAmbiguousBuiltinErr(builtin.second);
  }
}