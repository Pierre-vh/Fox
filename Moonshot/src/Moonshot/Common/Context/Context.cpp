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

void Context::setLoggingMode(const ContextLoggingMode & newmode)
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
	curstate_ = ContextState::UNSAFE;
}

void Context::reportFatalError(const std::string & message)
{
	addLog(
		makeLogMessage("FATAL", message)
	);
	// update state
	curstate_ = ContextState::CRITICAL;
}

ContextState Context::getState() const
{
	return curstate_;
}

void Context::resetState()
{
	curstate_ = ContextState::SAFE;
	//logs_.push_back("[Context] The context's state has been reset.");
}

BuildMode Context::getBuildMode() const
{
	return curbuildmode_;
}

void Context::setBuildMode(const BuildMode & newbuildmode)
{
	curbuildmode_ = newbuildmode;
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

void Context::clearLogs()
{
	logs_.clear();
}

void Context::addLog(const std::string & message)
{
	switch (curmode_)
	{
		case ContextLoggingMode::SILENT:
			break;
		case ContextLoggingMode::DIRECT_PRINT_AND_SAVE_TO_VECTOR:
			std::cout << message << std::endl;
			logs_.push_back(message);
			break;
		case ContextLoggingMode::DIRECT_PRINT:
			std::cout << message << std::endl;
			break;
		case ContextLoggingMode::SAVE_TO_VECTOR:
			logs_.push_back(message);
			break;
		default:
			throw std::logic_error("Defaulted : unknown ContextLoggingMode.");
			break;
	}
}

std::string Context::makeLogMessage(const std::string& prefix, const std::string & message) const
{
	std::stringstream out;
	if (prefix != "")
		out << '[' << prefix << "]\t";
	if (logsOrigin_ != "")
		out << '[' << logsOrigin_ << "]\t";
	out << message;
	return out.str();
}
