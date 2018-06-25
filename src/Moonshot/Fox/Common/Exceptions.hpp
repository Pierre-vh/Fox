////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Exceptions.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file declares various custom exceptions.										
////------------------------------------------------------////

#pragma once

#include <stdexcept> // basic exceptions : out_of_range etc. so they're imported with this file + exception header
#include <string> // std::string

namespace fox
{
	namespace Exceptions
	{
		class lexer_critical_error : public std::exception
		{
			public:
				inline lexer_critical_error(const std::string& msg = "")
				{
					msg_ += "\n" + msg;
				}
				virtual const char* what() const throw()
				{
					return msg_.c_str();
				}
			private:
				std::string msg_ = "The Lexer was in a corrupted state.";
		};

		class parser_critical_error : public std::exception
		{
			public:
				inline parser_critical_error(const std::string& msg = "")
				{
					msg_ += "\n" + msg;
				}
				virtual const char* what() const throw()
				{
					return msg_.c_str();
				}
			private:
				std::string msg_ = "The Parser encountered a critical error and had to terminate.";
		};

		class ast_malformation : public std::exception
		{
			public:
				inline ast_malformation(const std::string& msg = "")
				{
					msg_ += "\n" + msg;
				}
				virtual const char* what() const throw()
				{
					return msg_.c_str();
				}
			private:
				std::string msg_ = "A Visitor noticed that the AST was ill-formed.";
		};
	}
}