////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Context.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file contains the "Compilation Context".
// The compilation context owns a lot of things, like the
// DiagnosticsEngine, FlagsManager, etc.
////------------------------------------------------------////


#pragma once

#include <vector> // std::vector
#include "Moonshot/Common/Flags/FlagsManager.hpp"

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

			FlagsManager& flagsManager();
		private:
			FlagsManager flagsManager_;

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

