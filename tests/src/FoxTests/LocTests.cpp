////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : LocTests.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//	(Unit) Tests for the accuracy of Locations/Ranges of nodes in the AST.
////------------------------------------------------------////

#include "gtest/gtest.h"
#include "TestUtils/TestUtils.hpp"

#include "Moonshot/Fox/Parser/Parser.hpp"
#include "Moonshot/Fox/Lexer/Lexer.hpp"
#include "Moonshot/Fox/AST/Decl.hpp"
#include "Moonshot/Fox/Common/Context.hpp"
#include "Moonshot/Fox/Common/SourceManager.hpp"

using namespace Moonshot;

// Tests the accuracy of function SourceLocs: FuncDecl and ArgDecl 
TEST(LocTests, Functions)
{
	Context ctxt;
	ASTContext astctxt;
	DeclRecorder declrec;
	auto fid = ctxt.sourceManager.loadFromFile(Tests::convertRelativeTestResPathToAbsolute("astlocs/functions.fox"));
	ASSERT_TRUE(fid) << "could not load file";

	Lexer lex(ctxt, astctxt);
	lex.lexFile(fid);
	ASSERT_TRUE(ctxt.isSafe()) << "lexing error";

	Parser parser(ctxt, astctxt, lex.getTokenVector(), &declrec);
	auto presult = parser.parseFunctionDecl();

	ASSERT_TRUE(presult) << "parsing error";
	auto func = presult.moveAs<FunctionDecl>();

	// First, test the function itself
	CompleteLoc func_beg = ctxt.sourceManager.getCompleteLocForSourceLoc(func->getBegLoc());
	CompleteLoc func_head_end = ctxt.sourceManager.getCompleteLocForSourceLoc(func->getHeaderEndLoc());
	CompleteLoc func_end = ctxt.sourceManager.getCompleteLocForSourceLoc(func->getEndLoc());

	// Lines
	EXPECT_EQ(func_beg.line, 1);
	EXPECT_EQ(func_head_end.line, 1);
	EXPECT_EQ(func_end.line, 4);

	// Column & char
	EXPECT_EQ(func_beg.column, 1);
	EXPECT_EQ(func_beg.character, 1);

	EXPECT_EQ(func_head_end.column, 91);
	EXPECT_EQ(func_head_end.character, 91);

	EXPECT_EQ(func_end.column, 5);
	EXPECT_EQ(func_end.character, 2);

	// Value
	EXPECT_EQ(func_beg.value, 'f');
	EXPECT_EQ(func_head_end.value, 't');
	EXPECT_EQ(func_end.value, '}');

	// Now, test the args
	// Arg count should be correct
	ASSERT_EQ(func->argsSize(), 4);

	// Extract each arg individually
	ArgDecl* arg1 = func->getArg(0);
	ArgDecl* arg2 = func->getArg(1);
	ArgDecl* arg3 = func->getArg(2);
	ArgDecl* arg4 = func->getArg(3);

	// Check if the names are right
	EXPECT_EQ(arg1->getIdentifier()->getStr(), "_bar1");
	EXPECT_EQ(arg2->getIdentifier()->getStr(), "_bar2");
	EXPECT_EQ(arg3->getIdentifier()->getStr(), "_bar3");
	EXPECT_EQ(arg4->getIdentifier()->getStr(), "_bar4");

	// Extract all locs
	#define BEG_LOC(x) ctxt.sourceManager.getCompleteLocForSourceLoc(x->getBegLoc())
	#define END_LOC(x) ctxt.sourceManager.getCompleteLocForSourceLoc(x->getEndLoc())
	
	auto arg1_beg = BEG_LOC(arg1);
	auto arg1_end = END_LOC(arg1);

	auto arg2_beg = BEG_LOC(arg2);
	auto arg2_end = END_LOC(arg2);

	auto arg3_beg = BEG_LOC(arg3);
	auto arg3_end = END_LOC(arg3);

	auto arg4_beg = BEG_LOC(arg4);
	auto arg4_end = END_LOC(arg4);

	#undef BEG_LOC
	#undef END_LOC

	// Lines
	EXPECT_EQ(arg1_beg.line, 1);
	EXPECT_EQ(arg1_end.line, 1);

	EXPECT_EQ(arg2_beg.line, 1);
	EXPECT_EQ(arg2_end.line, 1);

	EXPECT_EQ(arg3_beg.line, 1);
	EXPECT_EQ(arg3_end.line, 1);

	EXPECT_EQ(arg4_beg.line, 1);
	EXPECT_EQ(arg4_end.line, 1);

	// Column
	EXPECT_EQ(arg1_beg.column, 10);
	EXPECT_EQ(arg1_end.column, 31);

	EXPECT_EQ(arg2_beg.column, 34);
	EXPECT_EQ(arg2_end.column, 50);

	EXPECT_EQ(arg3_beg.column, 53);
	EXPECT_EQ(arg3_end.column, 67);

	EXPECT_EQ(arg4_beg.column, 70);
	EXPECT_EQ(arg4_end.column, 82);

	// Chars
	EXPECT_EQ(arg1_beg.character, 10);
	EXPECT_EQ(arg1_end.character, 31);

	EXPECT_EQ(arg2_beg.character, 34);
	EXPECT_EQ(arg2_end.character, 50);

	EXPECT_EQ(arg3_beg.character, 53);
	EXPECT_EQ(arg3_end.character, 67);

	EXPECT_EQ(arg4_beg.character, 70);
	EXPECT_EQ(arg4_end.character, 82);

	// Values
	EXPECT_EQ(arg1_beg.value, '_');
	EXPECT_EQ(arg1_end.value, ']');

	EXPECT_EQ(arg2_beg.value, '_');
	EXPECT_EQ(arg2_end.value, 't');

	EXPECT_EQ(arg3_beg.value, '_');
	EXPECT_EQ(arg3_end.value, 't');

	EXPECT_EQ(arg4_beg.value, '_');
	EXPECT_EQ(arg4_end.value, 'g');
}

TEST(LocTests, VarDecls)
{
	Context ctxt;
	ASTContext astctxt;
	DeclRecorder declrec;
	auto fid = ctxt.sourceManager.loadFromFile(Tests::convertRelativeTestResPathToAbsolute("astlocs/vardecl.fox"));
	ASSERT_TRUE(fid) << "could not load file";

	Lexer lex(ctxt, astctxt);
	lex.lexFile(fid);
	ASSERT_TRUE(ctxt.isSafe()) << "lexing error";

	Parser parser(ctxt, astctxt, lex.getTokenVector(), &declrec);
	auto presult = parser.parseVarDecl();

	ASSERT_TRUE(presult) << "parsing error";
	auto var = presult.moveAs<VarDecl>();

	CompleteLoc var_beg = ctxt.sourceManager.getCompleteLocForSourceLoc(var->getBegLoc());
	CompleteLoc var_end = ctxt.sourceManager.getCompleteLocForSourceLoc(var->getEndLoc());

	EXPECT_EQ(var_beg.line, 1);
	EXPECT_EQ(var_end.line, 2);

	EXPECT_EQ(var_beg.column, 13);
	EXPECT_EQ(var_end.column, 8);

	EXPECT_EQ(var_beg.character, 4);
	EXPECT_EQ(var_end.character, 5);

	EXPECT_EQ(var_beg.value, 'l');
	EXPECT_EQ(var_end.value, ';');
}