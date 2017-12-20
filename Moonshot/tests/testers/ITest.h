#pragma once

#include "../../src/Moonshot/Fox/Parser/Parser.h"
#include "../../src/Moonshot/Fox/Lexer/Lexer.h"

#include "../../src/Moonshot/Fox/AST/Visitor/Dumper/Dumper.h"

#include <iostream>
#include <sstream>
#include <fstream>

class ITest
{
	public:
		void setFilePath(const std::string &fp);
		std::string getFilePath() const;

		bool testFile(const std::string & fp, const bool & shouldFail);

		virtual bool testStr(const std::string &str, const bool &shouldFailTC = false) = 0;
		virtual bool run() = 0;

		std::string fp_ = "";
		bool showAST = false;
};

