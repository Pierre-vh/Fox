#pragma once

#include <string>

class ITest
{
	public:
		void setFilePath(const std::string &fp);
		std::string getFilePath() const;
		virtual bool run() = 0;
	protected:
		std::string fp_ = "";
};

