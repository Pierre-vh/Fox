
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

using namespace Moonshot;

std::string Moonshot::dumpFVal(const FVal & var)
{
	std::stringstream ss;
	ss << "VariantIndex:" << var.index() << " ";
	if (std::holds_alternative<int>(var))
		ss << " [INT] Value : " << std::get<int>(var);
	else if (std::holds_alternative<float>(var))
		ss << " [FLOAT] Value : " << std::get<float>(var);
	else if (std::holds_alternative<std::string>(var))
		ss << " [STRING] Value : \"" << std::get<std::string>(var) << "\"";
	else if (std::holds_alternative<bool>(var))
		ss << " [BOOL] Value : " << std::get<bool>(var);
	else if (std::holds_alternative<char>(var))
		ss << " [CHAR] Value : " << std::get<char>(var);
	else
		E_CRITICAL("Illegal variant.");
	return ss.str();
}

FVal performOp(const FVal & left, const FVal & right)
{
	// to do, perform operation on FVals and return another fval with the result.
	return FVal();
}
