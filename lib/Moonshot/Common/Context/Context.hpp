////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Context.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This class declares a "Context" class used to track the current state of the interpreter, along with
// other parameters !
//
// SAFE -> No Warning and No Errors
// WARNING -> Must be used for errors that do not perturbate the interpretation process.
// UNSAFE -> Used for normal errors. e.g. "Undeclared variable x",etc..
//
// This class also stores a public Flag and Option member to access the current context's flags and options.
////------------------------------------------------------////


#pragma once

#include <vector> // std::vector
#include "Flags\Flags.hpp"

namespace Moonshot
{
	class Context
	{
		public:
			// Enums
			enum class State
			{
				SAFE,
				WARNING,
				UNSAFE,
				CRITICAL
			};
			enum class LoggingMode
			{
				DIRECT_PRINT_AND_SAVE_TO_VECTOR,
				DIRECT_PRINT,
				SAVE_TO_VECTOR,
				SILENT
			};
			enum class BuildMode
			{
				RELEASE, DEBUG
			};
			// Default ctor
			Context() = default;
			Context(const LoggingMode& lm);

			void setLoggingMode(const LoggingMode& newmode); // set mode : direct print to cout (default) or save to a vector.
			
			// logs are of the following form : [LOG/WARNING/UNSAFE][ORIGIN] Message
			void setOrigin(const std::string& origin);
			void resetOrigin();

			void logMessage(const std::string& message);
			void reportWarning(const std::string& message);
			void reportError(const std::string& message);
			void reportFatalError(const std::string& message);

			State getState() const;
			void resetState();

			BuildMode getBuildMode() const;
			void setBuildMode(const BuildMode& newbuildmode);

			void printLogs() const;		// print all logs to cout
			std::string getLogs() const; // returns a string containing the error log.
			void clearLogs();

			// issafe
			bool isCritical() const;
			bool isSafe() const;

			// TODO : Find a better way to let other classes access the flags without making the flags public
			// or returning a reference to a private member.
				// Who should be able to read this variable ? Everyone.
				// Who should be able to write to this variable ? Only a few classes.
			// Potential solution #1 :
				// Make flags protected so friends can access it
				//  and add a "const Flags& getFlags() const" method that's public for the other classes to read flags.
			// Potential solution #2 : Change how the flag struct works, but that's not a good option imo. 
			// I want to keep that struct as simple as possible.
			const FoxFlags& getFoxFlags() const;
			const CommonFlags& getCommonFlags() const;
		protected:
			FoxFlags foxflags_;
			CommonFlags comflags_;
		private:
			void addLog(const std::string& message);
			std::string makeLogMessage(const std::string& prefix, const std::string & message)const;

			// Make the context uncopyable and unassignable. It needs to be unique!
			Context(const Context&) = delete;      
			void operator=(const Context&) = delete;
			
			std::string logsOrigin_;
			std::vector<std::string> logs_;

			LoggingMode curmode_ = LoggingMode::DIRECT_PRINT_AND_SAVE_TO_VECTOR;
			State curstate_ = State::SAFE;
			BuildMode curbuildmode_ = BuildMode::DEBUG;
	};
}

