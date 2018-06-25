////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : TestUtils.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "TestUtils.hpp"

#include <fstream>
#include "Moonshot/Fox/Common/Utils.hpp"
using namespace fox;



const std::string testsPath = std::string(TEST_RES_PATH) + std::string("/res/");

bool test::readFileToVec(const std::string & filepath, std::vector<std::string>& outvec)
{
	std::ifstream in(convertRelativeTestResPathToAbsolute(filepath), std::ios::in | std::ios::binary); 	// open file
	std::string str; 	// temp str
	if (!in)
		return false;

	while (getline(in, str))
		outvec.push_back(str);
	return true;
}

bool test::readFileToString(const std::string & filepath, std::string & outstr)
{
	std::ifstream in(convertRelativeTestResPathToAbsolute(filepath), std::ios::binary); 	// read file
	if (in)
	{
		outstr = (std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()));
		return true;
	}
	return false;
}

std::string test::convertRelativeTestResPathToAbsolute(const std::string & relpath)
{
	return testsPath + relpath;
}

std::string test::indent(const unsigned char & size)
{
	std::string out;
	for (unsigned char k(0); k < size; k++)
		out += '\t';
	return out;
}
