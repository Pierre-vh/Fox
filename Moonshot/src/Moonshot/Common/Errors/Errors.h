////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Errors.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file declares the Error Singleton Class and Various macros used
// for error reporting.
////------------------------------------------------------////

#pragma once

#include <iostream> // cerr
#include <string>	// << std::string <<
#include "../Options.h"
#include "../Macros.h"

#define E_LOG(y)		Moonshot::Errors::getInstance().logInfo(y)
#define E_WARNING(y)	Moonshot::Errors::getInstance().reportWarning	(__FILE__,__LINE__,y)
#define E_ERROR(y)		Moonshot::Errors::getInstance().reportError	(__FILE__,__LINE__,y)
#define E_CRITICAL(y)	Moonshot::Errors::getInstance().reportCritical	(__FILE__,__LINE__,y)

#define E_CHECKSTATE	Moonshot::Errors::getInstance()

#define E_GETSTATE		Moonshot::Errors::getInstance().getCurrentState()
#define E_GETSTATE_STR	Moonshot::Errors::getInstance().getCurrentState()

#define E_RESETSTATE	Moonshot::Errors::getInstance().resetStatus()

#define E_RESET_ERROR_CONTEXT  Moonshot::Errors::getInstance().resetContext()
#define E_SET_ERROR_CONTEXT(x) Moonshot::Errors::getInstance().updateContext(x);

namespace Moonshot
{
	enum errstate 
	{
		GOOD,
		WARNING,
		ERROR,
		CRITICAL 
	};

	// SINGLETON
	class Errors
	{
		public:

			static Errors& getInstance();	 // get the instance

			inline void updateContext(const std::string &c_tag) // Update the "context" tag. (the name in square brackets before the errors.
			{
				context_ = "[" + c_tag + "]";
			}
			inline void resetContext() // Reset the context (don't use one anymore.)
			{
				context_ = "";
			}

			void logInfo(const std::string &str);

			void reportWarning(const char *file, int line, const std::string &txt);	// Warnings
			void reportWarning(const std::string &txt);	// Warning
			void reportError(const char *file, int line, const std::string &txt);	// Errors that disrupt the interpretation process without being too grave. (Semantic,Syntaxic error,etc);
			void reportCritical(const char *file, int line, const std::string &txt); // CRITICAL ERRORS : Errors that should never happen in normal condition, either you're doing something very wrong when using Moonshot, or Moonshot has a bug !
		
			errstate getCurrentState() const;
			std::string getCurrentState_asStr() const;

			void resetStatus();

			operator bool() const;

			struct options_ 
			{
				void muteAll(const bool &b);
				bool muteLogs		= false;
				bool muteWarnings	= false;
				bool muteErrors		= false;
				bool muteCriticals	= false;
			}options;

			~Errors();
		private:
			Errors();							 // Prevent instancing
			DISALLOW_COPY_AND_ASSIGN(Errors)

			// Attributes
			std::string context_ = "";

			errstate state_ = GOOD;
	};
}
