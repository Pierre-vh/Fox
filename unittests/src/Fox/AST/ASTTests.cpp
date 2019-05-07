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

// Tests that primitive types can be retrieve correctly
TEST_F(ASTTest, PrimitiveTypes) {
  using PT = PrimitiveType;
  using PTK = PT::Kind;

  PT* primBool = PT::getBool(ctxt);
  PT* primFloat  = PT::getDouble(ctxt);
  PT* primInt  = PT::getInt(ctxt);
  PT* primChar  = PT::getChar(ctxt);
  PT* primString = PT::getString(ctxt);
  PT* primVoid  = PT::getVoid(ctxt);

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
  EXPECT_EQ(primFloat->getPrimitiveKind(), PTK::DoubleTy);
  EXPECT_TRUE(primFloat->isDoubleType());

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
  EXPECT_EQ(primVoid, PT::getVoid(ctxt));
  EXPECT_EQ(primInt, PT::getInt(ctxt));
  EXPECT_EQ(primString, PT::getString(ctxt));
  EXPECT_EQ(primChar, PT::getChar(ctxt));
  EXPECT_EQ(primFloat, PT::getDouble(ctxt));
  EXPECT_EQ(primBool, PT::getBool(ctxt));
}

TEST_F(ASTTest, ASTContextArrayTypes) {
  Type primBool = PrimitiveType::getBool(ctxt);
  Type primFloat = PrimitiveType::getDouble(ctxt);
  Type primInt = PrimitiveType::getInt(ctxt);
  Type primChar = PrimitiveType::getChar(ctxt);
  Type primString = PrimitiveType::getString(ctxt);

  ArrayType* boolArr = ArrayType::get(ctxt, primBool);
  ArrayType* floatArr = ArrayType::get(ctxt, primFloat);
  ArrayType* intArr = ArrayType::get(ctxt, primInt);
  ArrayType* charArr = ArrayType::get(ctxt, primChar);
  ArrayType* strArr = ArrayType::get(ctxt, primString);


  // Check that pointers aren't null
  ASSERT_TRUE(boolArr)  << "Pointer is null";
  ASSERT_TRUE(floatArr)  << "Pointer is null";
  ASSERT_TRUE(intArr)    << "Pointer is null";
  ASSERT_TRUE(charArr)  << "Pointer is null";
  ASSERT_TRUE(strArr)    << "Pointer is null";

  // Check that itemTypes are correct
  EXPECT_EQ(boolArr->getElementType(), primBool);
  EXPECT_EQ(floatArr->getElementType(), primFloat);
  EXPECT_EQ(intArr->getElementType(), primInt);
  EXPECT_EQ(charArr->getElementType(), primChar);
  EXPECT_EQ(strArr->getElementType(), primString);

  // Checks that they're different
  EXPECT_NE(boolArr, floatArr);
  EXPECT_NE(floatArr, intArr);
  EXPECT_NE(intArr, charArr);
  EXPECT_NE(charArr, strArr);

  // Check that uniqueness works by getting the arraytype for int 
  EXPECT_EQ(ArrayType::get(ctxt, primInt), intArr);
}

TEST_F(ASTTest, TypeRTTI) {
  TypeBase* intTy = PrimitiveType::getInt(ctxt);
  TypeBase* arrIntTy = ArrayType::get(ctxt, intTy);
  TypeBase* lvIntTy = LValueType::get(ctxt, intTy);
  TypeBase* errType = ErrorType::get(ctxt);
  TypeBase* tyVarType = TypeVariableType::create(ctxt, 0);

  FunctionTypeParam p1(intTy, false);
  FunctionTypeParam p2(intTy, true);
  TypeBase* funcType = FunctionType::get(ctxt, {p1, p2}, intTy);

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

  EXPECT_EQ(funcType->getKind(), TypeKind::FunctionType);
  EXPECT_TRUE(FunctionType::classof(funcType));

  EXPECT_EQ(tyVarType->getKind(), TypeKind::TypeVariableType);
  EXPECT_TRUE(TypeVariableType::classof(tyVarType));
}

TEST_F(ASTTest, ExprRTTI) {
  // Binary Exprs
  auto* binexpr = BinaryExpr::create(ctxt, BinaryExpr::OpKind::Invalid,
    nullptr, nullptr, SourceRange());
  EXPECT_EQ(binexpr->getKind(), ExprKind::BinaryExpr);
  EXPECT_TRUE(BinaryExpr::classof(binexpr));

  // Unary Exprs
  auto* unaryexpr = UnaryExpr::create(ctxt, UnaryExpr::OpKind::Invalid, 
    nullptr, SourceRange());
  EXPECT_EQ(unaryexpr->getKind(), ExprKind::UnaryExpr);
  EXPECT_TRUE(UnaryExpr::classof(unaryexpr));

  // Cast Exprs
  auto* castexpr = CastExpr::create(ctxt, TypeLoc(), nullptr);
  EXPECT_EQ(castexpr->getKind(), ExprKind::CastExpr);
  EXPECT_TRUE(CastExpr::classof(castexpr));

  // Literals
  auto* charlit = CharLiteralExpr::create(ctxt, '0', SourceRange());
  EXPECT_EQ(charlit->getKind(), ExprKind::CharLiteralExpr);
  EXPECT_TRUE(CharLiteralExpr::classof(charlit));

  auto* intlit = IntegerLiteralExpr::create(ctxt, 0, SourceRange());
  EXPECT_EQ(intlit->getKind(), ExprKind::IntegerLiteralExpr);
  EXPECT_TRUE(IntegerLiteralExpr::classof(intlit));

  auto* floatlit = DoubleLiteralExpr::create(ctxt, 0.0, SourceRange());
  EXPECT_EQ(floatlit->getKind(), ExprKind::DoubleLiteralExpr);
  EXPECT_TRUE(DoubleLiteralExpr::classof(floatlit));

  auto* strlit = StringLiteralExpr::create(ctxt, string_view(), SourceRange());
  EXPECT_EQ(strlit->getKind(), ExprKind::StringLiteralExpr);
  EXPECT_TRUE(StringLiteralExpr::classof(strlit));

  auto* boollit = BoolLiteralExpr::create(ctxt, false, SourceRange());
  EXPECT_EQ(boollit->getKind(), ExprKind::BoolLiteralExpr);
  EXPECT_TRUE(BoolLiteralExpr::classof(boollit));

  auto* arrlit = ArrayLiteralExpr::create(ctxt, (ExprVector()), SourceRange());
  EXPECT_EQ(arrlit->getKind(), ExprKind::ArrayLiteralExpr);
  EXPECT_TRUE(ArrayLiteralExpr::classof(arrlit));

  auto* undeclref = UnresolvedDeclRefExpr::create(ctxt,
    (Identifier()), SourceRange());
  EXPECT_EQ(undeclref->getKind(), ExprKind::UnresolvedDeclRefExpr);
  EXPECT_TRUE(UnresolvedDeclRefExpr::classof(undeclref));

  auto* declref = DeclRefExpr::create(ctxt, nullptr, SourceRange());
  EXPECT_EQ(declref->getKind(), ExprKind::DeclRefExpr);
  EXPECT_TRUE(DeclRefExpr::classof(declref));

  auto* membof = MemberOfExpr::create(ctxt, nullptr, Identifier(),
    SourceRange(), SourceLoc());
  EXPECT_EQ(membof->getKind(), ExprKind::MemberOfExpr);
  EXPECT_TRUE(MemberOfExpr::classof(membof));

  auto* arracc = ArraySubscriptExpr::create(ctxt, nullptr, nullptr, SourceLoc());
  EXPECT_EQ(arracc->getKind(), ExprKind::ArraySubscriptExpr);
  EXPECT_TRUE(ArraySubscriptExpr::classof(arracc));

  auto* callexpr = CallExpr::create(ctxt, nullptr,
    ExprVector(), SourceLoc());
  EXPECT_EQ(callexpr->getKind(), ExprKind::CallExpr);
  EXPECT_TRUE(CallExpr::classof(callexpr));

  auto* errExpr = ErrorExpr::create(ctxt);
  EXPECT_EQ(errExpr->getKind(), ExprKind::ErrorExpr);
  EXPECT_TRUE(ErrorExpr::classof(errExpr));
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
    return VarDecl::create(ctxt, dc, Identifier(), SourceRange(), TypeLoc(), 
      VarDecl::Keyword::Let, nullptr, SourceRange());
  }

  FuncDecl* createEmptyFnDecl(ASTContext& ctxt, UnitDecl* unit) {
    return FuncDecl::create(ctxt, unit, SourceLoc(), Identifier(),
      SourceRange(), nullptr, TypeLoc());
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
  Type intTy = PrimitiveType::getInt(ctxt);
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
  Type intTy = PrimitiveType::getInt(ctxt);
  Type boolTy = PrimitiveType::getBool(ctxt);
  Type voidTy = PrimitiveType::getVoid(ctxt);

  FunctionTypeParam p1(intTy, false);
  FunctionTypeParam p2(boolTy, true);

  Type fns[6];
  // Create a few functions with different signatures.
  // (int, mut bool) -> void
  fns[0] = FunctionType::get(ctxt, {p1, p2}, voidTy);
  // (int) -> bool
  fns[1] = FunctionType::get(ctxt, {p1}, boolTy);
  // (mut bool) -> int
  fns[2] = FunctionType::get(ctxt, {p2}, intTy);
  // () -> void
  fns[3] = FunctionType::get(ctxt, {}, voidTy);
  // (mut bool, int) -> void
  fns[4] = FunctionType::get(ctxt, {p2, p1}, voidTy);
  // (mut bool) -> void
  fns[5] = FunctionType::get(ctxt, {p2}, voidTy);

  // Check that they all have different pointers
  std::set<TypeBase*> ptrs;
  for(std::size_t k = 0; k < 6; k++) {
    auto result = ptrs.insert(fns[k].getPtr());
    EXPECT_TRUE(result.second) << "element already exists";
  }

  // Check that we can successfully retrieve every function type
  // while keeping the same pointer value.
  EXPECT_EQ(fns[0], FunctionType::get(ctxt, {p1, p2}, voidTy));
  EXPECT_EQ(fns[1], FunctionType::get(ctxt, {p1}, boolTy));
  EXPECT_EQ(fns[2], FunctionType::get(ctxt, {p2}, intTy));
  EXPECT_EQ(fns[3], FunctionType::get(ctxt, {}, voidTy));
  EXPECT_EQ(fns[4], FunctionType::get(ctxt, {p2, p1}, voidTy));
  EXPECT_EQ(fns[5], FunctionType::get(ctxt, {p2}, voidTy));

  // Also, check that 'mut' taken into account
  FunctionTypeParam p2immut(boolTy, false);
  EXPECT_NE(FunctionType::get(ctxt, {p2immut}, voidTy), 
            FunctionType::get(ctxt, {p2}, voidTy));
}

class ASTBuiltinsTest : public ::testing::Test {
  public:
    ASTBuiltinsTest() : diags(srcMgr, std::cout), ctxt(srcMgr, diags) {}

    SourceManager srcMgr;
    DiagnosticEngine diags;
    ASTContext ctxt;

};

TEST_F(ASTBuiltinsTest, getTypeOfBuiltinTest) {
  Type printIntTy = ctxt.getTypeOfBuiltin(BuiltinID::printInt);
  Type printBoolTy = ctxt.getTypeOfBuiltin(BuiltinID::printBool);
  Type printStrTy = ctxt.getTypeOfBuiltin(BuiltinID::printString);

  // Is 'toDebugString' fine for this?
  EXPECT_EQ(printIntTy->toDebugString(),    "(int) -> void");
  EXPECT_EQ(printBoolTy->toDebugString(),   "(bool) -> void");
  EXPECT_EQ(printStrTy->toDebugString(),    "(string) -> void");
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
  #define BUILTIN(FUNC, FOX)\
    buitlins[#FOX].push_back({#FUNC, ctxt.getTypeOfBuiltin(BuiltinID::FUNC)});
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