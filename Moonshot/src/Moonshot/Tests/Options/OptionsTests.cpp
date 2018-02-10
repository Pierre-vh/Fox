////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : OptionsTests.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "OptionsTests.hpp"

#include <sstream>

using namespace Moonshot;

OptionsTests::OptionsTests()
{
}

OptionsTests::~OptionsTests()
{
}

std::string OptionsTests::getTestName() const
{
	return "OptionsManager and ParameterValue unit tests.";
}

bool OptionsTests::runTest(Context & context)
{
	auto options = context.optionsManager_; // Copy it so we don't modify the context's one
	if (!testOptManagerFunc(context, options))
		return false;
	if (!testParamvalueFuncs(context))
		return false;
	return true;
}

bool OptionsTests::testOptManagerFunc(Context & context, OptionsManager & options)
{
	// Tests if the added attr's value is added correctly and preserved.
	const OptionsList optname = OptionsList::TESTERCLASS_TESTOPT;
	ParameterValue pval(46846886);
	// store the boolean : ADD
	options.addAttr(optname, pval);
	// has attribute ? HAS
	if (!options.hasAttr(optname))
	{
		context.reportError("OptionsTest::testOptManagerFunc -> Failed. hasAttr returned false after insertion. addAttr or hasAttr is broken!");
		return false;
	}
	// get attribute and compare : GET
	if (pval != options.getAttr(optname))
	{
		context.reportError("OptionsTest::testOptManagerFunc -> Failed. getAttr returned a different value after insertion. getAttr or addAttr is broken!");
		return false;
	}
	// set attribute
	ParameterValue pval_bis(-1);
	options.setAttr(optname, pval_bis);
	// check if we still have it
	if (!options.hasAttr(optname))
	{
		context.reportError("OptionsTest::testOptManagerFunc -> Failed. hasAttr returned false after setAttr. setAttr or hasAttr is broken!");
		return false;
	}
	// check if it's still ok
	if (pval_bis != options.getAttr(optname))
	{
		context.reportError("OptionsTest::testOptManagerFunc -> Failed. getAttr returned a different value after setAttr. getAttr,setAttr or operator!= is broken!");
		return false;
	}
	// delete it
	options.deleteAttr(optname);
	// wow, we still have it in? broken.
	if (options.hasAttr(optname)) 
	{
		context.reportError("OptionsTest::testOptManagerFunc -> Failed. Called deleteAttr, but hasAttr still returned true. deleteAttr or hasAttr is broken.");
		return false;
	}
	// aaaand it's good
	return true;
}

bool OptionsTests::testParamvalueFuncs(Context & context)
{
	// test construction with different values
	// P1 = BOOl
	// P2 = INT
	// base values
	bool p1_expected = true;
	int p2_expected = -34520;
	// casted expected values
	int p1_casted_expected = 1;
	bool p2_casted_expected = true;
	ParameterValue p1(p1_expected), p2(p2_expected);
	// holds
	if (!p1.holdsType<bool>())
	{
		context.reportError("OptionsTests::testParamvalueFuncs -> p1.holdsType<bool> returned false when it should've returned true. Constructor or holdsType is broken");
		return false;
	}
	if (!p2.holdsType<int>())
	{
		context.reportError("OptionsTests::testParamvalueFuncs -> p2.holdsType<int> returned false when it should've returned true. Constructor or holdsType is broken");
		return false;
	}
	// get
	if (p1.get<bool>() != p1_expected)
	{
		context.reportError("OptionsTests::testParamvalueFuncs -> Failed. p1.get<bool> did not return the expected type. Constructor or getter function is broken!");
		return false;
	}
	if (p2.get<int>() != p2_expected)
	{
		context.reportError("OptionsTests::testParamvalueFuncs -> Failed. p2.get<int> did not return the expected type. Constructor or getter function is broken!");
		return false;
	}
	// operator ==
	if (p1 == p2)
	{
		context.reportError("OptionsTests::testParamvalueFuncs -> Failed. p1 == p2 returned true, but they're different values. operator== is broken!");
		return false;
	}
	// Operator !=
	if (!(p1 != p2))
	{
		context.reportError("OptionsTests::testParamvalueFuncs -> Failed. p1 != p2 returned false, but they're different values. operator!= is broken!");
		return false;
	}
	// test cast
	if (p1.get<int>() != p1_casted_expected)
	{
		std::stringstream out;
		out << "OptionsTests::testParamvalueFuncs -> Failed. Attempted to retrieve p1 as int, but the result wasn't the expected one. Cast function is broken ! Result : \xAE" << p1.get<int>() << "\xAF";
		context.reportError(out.str());
		return false;
	}
	if (p2.get<bool>() != p2_casted_expected)
	{
		context.reportError("OptionsTests::testParamvalueFuncs -> Failed. Attempted to retrieve p2 as bool, but the result wasn't the expected one. Cast function is broken !");
		return false;
	}
	// ok !
	return true;
}
