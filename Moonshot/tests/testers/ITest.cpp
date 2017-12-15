#include "ITest.h"

void ITest::setFilePath(const std::string & fp)
{
	fp_ = fp;
}

std::string ITest::getFilePath() const
{
	return fp_;
}
