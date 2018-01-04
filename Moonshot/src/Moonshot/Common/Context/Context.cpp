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

void Context::setMode(const ContextMode & newmode)
{
	curmode_ = newmode;
}

void Context::setOrigin(const std::string & origin)
{
	logsOrigin_ = origin;
}

void Context::resetOrigin()
{
	logsOrigin_ = "";
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

void Context::resetState()
{
	curstate_ = ContextState::GOOD;
	logs_.push_back("[Context] The context's state has been reset.");
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
	switch (curmode_)
	{
		case ContextMode::DIRECT_PRINT_AND_SAVE_TO_VECTOR:
			std::cout << message << std::endl;
			logs_.push_back(message);
			break;
		case ContextMode::DIRECT_PRINT:
			std::cout << message << std::endl;
			break;
		case ContextMode::SAVE_TO_VECTOR:
			logs_.push_back(message);
			break;
		default:
			throw std::logic_error("Defaulted : unknown ContextMode.");
			break;
	}
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
