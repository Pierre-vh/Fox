
/************************************************************
Author : Pierre van Houtryve
Contact :
e-mail : pierre.vanhoutryve@gmail.com

Description : typedef for FValue (in the future, helper functions will be added here.)

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

#include <variant> // std::variant
#include <string> // std::string
#include <sstream> // std::stringstream
#include <type_traits> // std::is_same

#include "../../Common/Errors/Errors.h"
#include "../../Fox/Util/Enums.h"



// Alias for a variant holding every type possible in the interpreter.
typedef std::variant<int, float, char, std::string, bool> FVal;

namespace Moonshot
{
	std::string dumpFVal(const FVal &var);
	FVal getSampleFValForIndex(const std::size_t& t);

	// Thanks, I guess ! This looks healthier than using -1 as invalid index. https://stackoverflow.com/a/37126153
	static constexpr std::size_t invalid_index = std::numeric_limits<std::size_t>::max();
	// How to remember values of index
	static constexpr std::size_t fval_int = 0;
	static constexpr std::size_t fval_float = 1;
	static constexpr std::size_t fval_char = 2;
	static constexpr std::size_t fval_str = 3;
	static constexpr std::size_t fval_bool = 4;

	const std::map<std::size_t, std::string> kType_dict =
	{
		{ fval_int				, "INT" },
		{ fval_float			, "FLOAT" },
		{ fval_char				, "CHAR" },
		{ fval_bool				, "BOOL" },
		{ fval_str				, "STRING" },
		{ invalid_index			, "!INVALID_FVAL!" }
	};

}