////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Exceptions.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file declares various custom exceptions.										
////------------------------------------------------------////

#pragma once

#include <exception> // std::exception
#include <stdexcept> // basic exceptions : out_of_range etc.;
#include <string> // std::string
#include <sstream>

namespace Moonshot
{
	namespace Exceptions
	{
		class lexer_critical_error : public std::exception
		{
			public:
				lexer_critical_error(const std::string& msg = "");
				virtual const char* what() const throw()
				{
					return msg_.c_str();
				}
			private:
				std::string msg_ = "The Lexer was in a corrupted state.";
		};
	}
}