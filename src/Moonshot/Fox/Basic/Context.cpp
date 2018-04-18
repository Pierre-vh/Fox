////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Context.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Context.hpp"
#include <iostream> // std::cout
#include <sstream> // std::stringstream

using namespace Moonshot;

Context::Context()
{
	
}

Context::Context(const LoggingMode & lm)
{
	setLoggingMode(lm);
}

void Context::setLoggingMode(const LoggingMode & newmode)
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
	if(curstate_ == State::SAFE)
		curstate_ = State::WARNING;
}

void Context::reportError(const std::string & message)
{
	addLog(
		makeLogMessage("ERROR",message)
	);
	// update state
	if(curstate_ == State::SAFE || curstate_ == State::WARNING)
		curstate_ = State::UNSAFE;
}

void Context::reportFatalError(const std::string & message)
{
	addLog(
		makeLogMessage("FATAL", message)
	);
	// update state
	if(curstate_ != State::CRITICAL)
		curstate_ = State::CRITICAL;
}

Context::State Context::getState() const
{
	return curstate_;
}

void Context::resetState()
{
	curstate_ = State::SAFE;
	//logs_.push_back("[Context] The context's state has been reset.");
}

Context::BuildMode Context::getBuildMode() const
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

bool Context::isCritical() const
{
	return curstate_ == State::CRITICAL;
}

bool Context::isSafe() const
{
	return (curstate_ == State::SAFE) || (curstate_ == State::WARNING);
}

void Context::addLog(const std::string & message)
{
	switch (curmode_)
	{
		case LoggingMode::SILENT:
			break;
		case LoggingMode::DIRECT_PRINT_AND_SAVE_TO_VECTOR:
			std::cout << message << std::endl;
			logs_.push_back(message);
			break;
		case LoggingMode::DIRECT_PRINT:
			std::cout << message << std::endl;
			break;
		case LoggingMode::SAVE_TO_VECTOR:
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
