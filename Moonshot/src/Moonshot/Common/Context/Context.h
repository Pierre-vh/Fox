////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Context.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This class declares a "Context" class used to track the current state of the interpreter, along with
// other parameters !
//
// SAFE -> No Warning and No Errors
// WARNING -> Must be used for errors that do not perturbate the interpretation process.
// UNSAFE -> Used for normal errors. e.g. "Undeclared variable x",etc..
//
// This class also uses OptionsManager to store options.
////------------------------------------------------------////


#pragma once

#include <vector> // std::vector
#include <memory> // std::shared_ptr

#include "Options\OptionsManager.h" 

// This is used to define the maximum errors you can have before the context goes critical.
// Can be changed @ runtime with
#define DEFAULT_MAX_TOLERATED_ERRORS 4

namespace Moonshot
{
	enum class ContextState
	{
		SAFE,
		WARNING,
		UNSAFE,
		CRITICAL
	};
	enum class ContextLoggingMode
	{
		DIRECT_PRINT_AND_SAVE_TO_VECTOR,
		DIRECT_PRINT,
		SAVE_TO_VECTOR,
		SILENT
	};
	enum class BuildMode
	{
		RELEASE,DEBUG
	};
	class Context
	{
		public:
			// Default ctor
			Context() = default;

			void setLoggingMode(const ContextLoggingMode& newmode); // set mode : direct print to cout (default) or save to a vector.
			
			// logs are of the following form : [LOG/WARNING/UNSAFE][ORIGIN] Message
			void setOrigin(const std::string& origin);
			void resetOrigin();

			void logMessage(const std::string& message);
			void reportWarning(const std::string& message);
			void reportError(const std::string& message);
			void reportFatalError(const std::string& message);

			ContextState getState() const;
			void resetState();

			BuildMode getBuildMode() const;
			void setBuildMode(const BuildMode& newbuildmode);

			void printLogs() const;		// print all logs to cout
			std::string getLogs() const; // returns a string containing the error log.
			void clearLogs();

			// issafe
			bool isCritical() const;
			bool isSafe_strict() const;
			bool isSafe() const;

			OptionsManager options; // The options manager.
		private:

			void addLog(const std::string& message);
			std::string makeLogMessage(const std::string& prefix, const std::string & message)const;

			// Make the context uncopyable and unassignable.
			Context(const Context&) = delete;      
			void operator=(const Context&) = delete;
			
			std::string logsOrigin_;
			std::vector<std::string> logs_;

			ContextLoggingMode curmode_ = ContextLoggingMode::DIRECT_PRINT_AND_SAVE_TO_VECTOR;
			ContextState curstate_ = ContextState::SAFE;
			BuildMode curbuildmode_ = BuildMode::DEBUG;
	};
}

