////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Utils.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Utils.hpp"

#include <fstream>
#include "Moonshot/Common/Utils/Utils.hpp"
using namespace Moonshot;

static const std::string testsPath = "./../Moonshot/res/tests/";

bool Tests::readFileToVec(const std::string & filepath, std::vector<std::string>& outvec)
{
	std::string completePath = std::string(testsPath) + filepath; 	// Get complete Path
	std::ifstream in(completePath, std::ios::in | std::ios::binary); 	// open file
	std::string str; 	// temp str
	if (!in)
		return false;

	while (getline(in, str))
		outvec.push_back(str);
	return true;
}

bool Tests::readFileToString(const std::string & filepath, std::string & outstr)
{
	std::string completePath = std::string(testsPath) + filepath; 	// Get complete Path
	std::ifstream in(completePath, std::ios::binary); 	// read file
	if (in)
	{
		outstr = (std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()));
		return true;
	}
	return false;
}

std::string Tests::indent(const unsigned char & size)
{
	std::string out;
	for (unsigned char k(0); k < size; k++)
		out += '\t';
	return out;
}
