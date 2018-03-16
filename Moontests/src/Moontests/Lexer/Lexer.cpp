#include "Moontests/pch/pch.h"
#include "Moonshot/Common/Context/Context.hpp"

using namespace Moonshot;

TEST(ContextTests, SampleTest) {
	Context ctxt;
	ctxt.reportFatalError("Fatal!");
	EXPECT_FALSE(ctxt.isSafe());
}