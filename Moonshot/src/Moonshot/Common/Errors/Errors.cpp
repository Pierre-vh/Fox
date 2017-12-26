#include "Errors.h"

using namespace Moonshot;

Errors::Errors()
{

}

Errors& Errors::getInstance()
{
	static Errors instance;
	return instance;
}

void Moonshot::Errors::logInfo(const std::string & str)
{
	if (!options.muteLogs)
		std::clog << "[LOG]\t" << str << std::endl;
}

void Errors::reportWarning(const char * file, int line, const std::string &txt)
{
	if (!options.muteWarnings)
	{
		if constexpr (MOONSHOT_DETAILED_ERRORS)
			std::cerr << "[WARNING][" << file << " @line " << line << "]\n" << context_ << txt << std::endl;
		else
			std::cerr << "[WARNING]" << context_  << txt << std::endl;
	}
	state_ = WARNING;
}

void Errors::reportWarning(const std::string &txt)
{
	if (!options.muteWarnings)
	{
		std::cerr << "[WARNING]" << context_  << txt << std::endl;
	}
	state_ = WARNING;
}

void Errors::reportError(const char * file, int line, const std::string &txt)
{
	state_ = ERROR;
	if (!options.muteErrors)
	{
		if constexpr (MOONSHOT_DETAILED_ERRORS)
			std::cerr << "[ERROR][" << file << " @line " << line << "]\n" << context_  << txt << std::endl;
		else 
			std::cerr << "[ERROR]" << context_ << txt << std::endl;
	}
}

void Errors::reportCritical(const char * file, int line, const std::string &txt)
{
	if (!options.muteCriticals)
	{
		if constexpr (MOONSHOT_DETAILED_ERRORS)
			std::cerr << "[CRITICAL][" << file << " @line " << line << "]\n" << context_   << txt << std::endl;
		else 
			std::cerr << "[CRITICAL]" << context_ << txt << std::endl;
	}
	state_ = CRITICAL;
}

errstate Moonshot::Errors::getCurrentState() const 
{
	return state_;
}

std::string Moonshot::Errors::getCurrentState_asStr() const
{
	switch (state_)
	{
		case GOOD:
			return "GOOD";
		case WARNING:
			return "WARNING";
		case ERROR:
			return "ERROR";
		case CRITICAL:
			return "CRITICAL";
		default:
			return "<DEFAULTED>";
	}
}

void Moonshot::Errors::resetStatus()
{
	state_ = GOOD;
}

Moonshot::Errors::operator bool() const
{
	return state_ == GOOD || state_ == WARNING;		// Return true if we are not in a error/critical state.
}

Errors::~Errors()
{
}

void Moonshot::Errors::options_::muteAll(const bool &b)
{
	muteLogs = b;
	muteWarnings = b;
	muteErrors = b;
	muteCriticals = b;
}
