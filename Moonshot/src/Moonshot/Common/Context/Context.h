////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Context.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This class declares a "Context" class used to track the current state of the interpreter.
//
// GOOD -> No Warning and No Errors
// WARNING -> Must be used for errors that do not perturbate the interpretation process.
// ERROR -> Used for normal errors. e.g. "Undeclared variable x",etc..
////------------------------------------------------------////

#pragma once

#include <iostream> // std::cout
#include <vector> // std::vector
#include <sstream> // std::stringstream

namespace Moonshot
{
	enum class ContextState
	{
		GOOD,
		WARNING,
		ERROR
	};
	class Context
	{
		public:
			Context() = default;

			inline bool isSafe_strict() const 
			{
				return curstate_ == ContextState::GOOD;
			}
			inline bool isSafe() const 
			{
				return (curstate_ == ContextState::GOOD) || (curstate_ == ContextState::WARNING);
			}

			// logs are of the following form : [LOG/WARNING/ERROR][ORIGIN] Message
			void setOrigin(const std::string& origin);

			void logMessage(const std::string& message);
			void reportWarning(const std::string& message);
			void reportError(const std::string& message);

			ContextState getState() const;

			void printLogs() const;		// print all logs to cout
			std::string getLogs() const; // returns a string containing the error log.
		private:
			void addLog(const std::string& message);
			std::string makeLogMessage(const std::string& prefix, const std::string & message)const;

			// Make the context uncopyable and unassignable.
			Context(const Context&) = delete;      
			void operator=(const Context&) = delete;
			
			std::string logsOrigin_;
			std::vector<std::string> logs_;
			ContextState curstate_ = ContextState::GOOD;

	};
}

