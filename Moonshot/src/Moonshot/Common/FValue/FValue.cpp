
/*************************************************************
MIT License

Copyright(c) 2017 Pierre van Houtryve

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*************************************************************/

#include "FValue.h"


std::string Moonshot::dumpFVal(const FVal & var)
{
	std::stringstream ss;
	if (std::holds_alternative<int>(var))
		ss << "Type : INT, Value : " << std::get<int>(var);
	else if (std::holds_alternative<float>(var))
		ss << "Type : FLOAT, Value : " << std::get<float>(var);
	else if (std::holds_alternative<std::string>(var))
		ss << "Type : STRING, Value : \"" << std::get<std::string>(var) << "\"";
	else if (std::holds_alternative<bool>(var))
	{
		bool v = std::get<bool>(var);
		ss << "Type : BOOL, Value : " << (v ? "true" : "false");
	}
	else if (std::holds_alternative<char>(var))
	{
		char x = std::get<char>(var);
		ss << "Type : CHAR, Value : " << (int)x <<  " = '" << x << "'";
	}
	else
		E_CRITICAL("Illegal variant.");
	return ss.str();
}
FVal Moonshot::getSampleFValForIndex(const std::size_t & t)
{
	switch (t)
	{
		case fval_int:
			return FVal((int)0);
		case fval_float:
			return FVal((float)0.0f);
		case fval_char:
			return FVal((char)0);
		case fval_str:
			return FVal(std::string(""));
		case fval_bool:
			return FVal((bool)false);
		case invalid_index:
			E_CRITICAL("Tried to get a sample FVal with an invalid index");
			return FVal();
		default:
			E_CRITICAL("Defaulted while attempting to return a sample FVal for an index. -> Unknown index. Unimplemented type?");
			return FVal();
	}
}

std::string Moonshot::indexToTypeName(const std::size_t & t)
{
	auto a = kType_dict.find(t);
	return (a != kType_dict.end()) ? a->second : "!IMPOSSIBLE_TYPE!";
}
