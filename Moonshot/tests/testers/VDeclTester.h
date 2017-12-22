#pragma once

#include "ITest.h"

class VDeclTester : public ITest
{
	public:
		VDeclTester();
		~VDeclTester();

		// Inherited via ITest
		virtual bool testStr(const std::string & str, const bool & shouldFailTC = false) override;
		virtual bool run() override;
};

