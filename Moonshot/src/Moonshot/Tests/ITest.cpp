////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ITest.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "ITest.hpp"

using namespace Moonshot;
using namespace Moonshot::Test;

std::string TestUtilities::readFileToString(Context& context,const std::string& fp)
{
	std::string completePath = std::string(Util::moonshotSrcPath) + fp; 	// Get complete Path
	std::ifstream in(completePath, std::ios::binary); 	// read file
	if (in)
		return(std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()));
	// if error :
	context.setOrigin("TestUtilities");
	context.reportError("Can't open file " + fp);
	context.resetOrigin();
	return "";
}

std::vector<std::string> TestUtilities::readFileToVec(Context & context, const std::string & fp)
{
	std::string completePath = std::string(Util::moonshotSrcPath) + fp; 	// Get complete Path
	std::ifstream in(completePath, std::ios::in | std::ios::binary); 	// open file
	std::vector<std::string> vec; 	//create vector
	std::string str; 	// temp str
	// fill vec
	if (!in)
	{
		context.setOrigin("TestUtilities");
		context.reportError("Can't open file " + fp);
		context.resetOrigin();
		return std::vector<std::string>();
	}
	while (getline(in, str))
		vec.push_back(str);
	return vec;
}

ITest::~ITest()
{

}

