////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Context.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Context.h"

using namespace Moonshot;

void Context::setOrigin(const std::string & origin)
{
	logsOrigin_ = origin;
}

void Context::logMessage(const std::string & message)
{
	addLog(
		makeLogMessage("LOG",message)
	);
}

void Context::reportWarning(const std::string & message)
{
	addLog(
		makeLogMessage("WARNING",message)
	);
	// update state
	curstate_ = ContextState::WARNING;
}

void Context::reportError(const std::string & message)
{
	addLog(
		makeLogMessage("ERROR",message)
	);
	// update state
	curstate_ = ContextState::ERROR;
}

ContextState Context::getState() const
{
	return curstate_;
}

void Context::printLogs() const
{
	std::cout << getLogs();
}

std::string Context::getLogs() const
{
	std::stringstream output;
	for (auto& elem : logs_)
	{
		output << elem << std::endl;
	}
	return output.str();
}

void Context::addLog(const std::string & message)
{
	logs_.push_back(message);
}

std::string Context::makeLogMessage(const std::string& prefix, const std::string & message) const
{
	std::string out;
	if (prefix != "")
		out += '[' + prefix + "]\t";
	if (logsOrigin_ != "")
		out += '[' + logsOrigin_ + ']';
	
	out += message;
	return out;
}
