#include "ITest.h"

void ITest::setFilePath(const std::string & fp)
{
	fp_ = fp;
}

std::string ITest::getFilePath() const
{
	return fp_;
}

bool ITest::testFile(const std::string & fp, const bool & shouldFail)
{
	if (!E_CHECKSTATE)
		return false;

	std::ifstream file(fp); // Open file
	if (!file)
		std::cout << "Couldn't open file " << fp_ << std::endl;
	std::string line;
	while (std::getline(file, line))
	{
		if (!testStr(line, shouldFail))
			return false;
	}
	return true;
}

bool ITest::standardTest(const std::string & str, std::vector<Moonshot::IVisitor>& ct_vis, std::vector<Moonshot::IRTVisitor>& rt_vis)
{
	std::cout << "STR:" << str << std::endl;
	// Iterate on visitors to run them on str
	return false;
}

