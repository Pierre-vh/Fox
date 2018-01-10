////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : OptionsManager.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "OptionsManager.h"

using namespace Moonshot;

OptionsManager::OptionsManager()
{
}


OptionsManager::~OptionsManager()
{
}

void OptionsManager::addAttr(const std::string & new_module_name, const std::string & new_attr_name, const ParameterValue& new_pval)
{
	parameters_.insert(
		std::make_pair(
			std::make_pair(
				new_module_name,
				new_attr_name
			),
			new_pval
		)
	);
}

void OptionsManager::setAttr(const std::string & module_name, const std::string & attr_name, const ParameterValue & new_pval)
{
	auto it = parameters_.find(std::make_pair(module_name,attr_name));
	if (it != parameters_.end())
		it->second = new_pval;
	else // attribute not found, add it !
		addAttr(module_name, attr_name, new_pval);
}

void OptionsManager::deleteAttr(const std::string & module_name, const std::string & attr_name)
{
	auto it = parameters_.find(std::make_pair(module_name, attr_name));
	if (it != parameters_.end()) // erase if element exists
		parameters_.erase(it);
}

bool OptionsManager::hasAttr(const std::string & module_name, const std::string & attr_name) const
{
	auto reqresult = parameters_.find(std::make_pair(module_name, attr_name));
	return (reqresult != parameters_.end());
}

ParameterValue OptionsManager::getAttr(const std::string & module_name, const std::string & attr_name) const
{
	if (hasAttr(module_name, attr_name)) // hasAttr will do the checks needed. If it returns true, the value exists.
		return parameters_.find(std::make_pair(module_name, attr_name))->second;
	else
		std::cout << "failed to retrieve attribute [" << module_name << "," << attr_name << "]" << std::endl;
	return ParameterValue(false); // return "false"
}
