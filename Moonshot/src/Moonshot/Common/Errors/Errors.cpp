/************************************************************
Author : Pierre van Houtryve
Contact :
e-mail : pierre.vanhoutryve@gmail.com

Description : See header.

*************************************************************
MIT License

Copyright (c) 2017 Pierre van Houtryve

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*************************************************************/

#include "Errors.h"

using namespace Moonshot;

Errors*	Errors::instance = 0;

Errors::Errors()
{
}

Moonshot::Errors::Errors(Errors const &)
{
}

Errors* Errors::getInstance()
{

	if (!instance)
		instance = new Errors;
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
		std::cerr << "[WARNING][" << file << " @line " << line << "]\n" << txt << std::endl;

	state_ = WARNING;
}

void Errors::reportWarning(const std::string &txt)
{
	if (!options.muteWarnings)
		std::cerr << "[WARNING] -> " << std::endl;

	state_ = WARNING;
}

void Errors::reportError(const char * file, int line, const std::string &txt)
{
	std::cerr << "[ERROR][" << file << " @line " << line << "]\n" << txt << std::endl;

	state_ = ERROR;
}

void Errors::reportCritical(const char * file, int line, const std::string &txt)
{
	std::cerr << "[CRITICAL][" << file << " @line " << line << "]\n" << txt << std::endl;

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

Moonshot::Errors::operator bool() const
{
	return state_ == GOOD || state_ == WARNING;		// Return true if we are not in a error/critical state.
}

Errors::~Errors()
{
}


