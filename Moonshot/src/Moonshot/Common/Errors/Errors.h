/************************************************************
Author : Pierre van Houtryve
Contact :
e-mail : pierre.vanhoutryve@gmail.com

Description : This file is the class used to report errors in the program through the cerr stream, using a singleton class.

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

#pragma once

#include <iostream> // cerr
#include <string>	// << std::string <<

#define E_LOG(y)		Moonshot::Errors::getInstance()->logInfo(y);
#define E_WARNING(y)	Moonshot::Errors::getInstance()->reportWarning	(__FILE__,__LINE__,y);
#define E_ERROR(y)		Moonshot::Errors::getInstance()->reportError	(__FILE__,__LINE__,y);
#define E_CRITICAL(y)	Moonshot::Errors::getInstance()->reportCritical	(__FILE__,__LINE__,y);

#define E_CHECKSTATE	*(Moonshot::Errors::getInstance())

#define E_GETSTATE		Moonshot::Errors::getInstance()->getCurrentState()
#define E_GETSTATE_STR	Moonshot::Errors::getInstance()->getCurrentState()

#define E_RESETSTATE	Moonshot::Errors::getInstance()->resetStatus();

// Debug defines

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

			static Errors* getInstance();	 // get the instance

			void logInfo(const std::string &str);

			void reportWarning(const char *file, int line, const std::string &txt);	// Warnings
			void reportWarning(const std::string &txt);	// Warnings

			void reportError(const char *file, int line, const std::string &txt);	// Errors that disrupt the interpretation process without being too grave. (Semantic,Syntaxic error,etc);
			
			void reportCritical(const char *file, int line, const std::string &txt); // CRITICAL ERRORS : Errors that should never happen in normal condition, either you're doing something very wrong when using Moonshot, or Moonshot has a bug !
		
			errstate getCurrentState() const;
			std::string getCurrentState_asStr() const;

			void resetStatus();

			operator bool() const;

			struct options_ 
			{
				void setAll(const bool &b);
				bool muteLogs		= false;
				bool muteWarnings	= false;
				bool muteErrors		= false;
				bool muteCriticals	= false;
			}options;

			~Errors();
		private:
			Errors();							 // Prevent instancing
			Errors(Errors const&);				 // Prevent Copying
			Errors& operator=(Errors const&) {}  // Prevent Assignement
			static Errors* instance;

			// Attributes

			errstate state_ = GOOD;
	};
}
