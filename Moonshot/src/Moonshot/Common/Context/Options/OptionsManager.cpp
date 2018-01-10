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

void OptionsManager::addAttr(const OptionsList& optname, const ParameterValue& new_pval)
{
	parameters_.insert(
		std::make_pair(
			optname,
			new_pval
		)
	);
}

void OptionsManager::setAttr(const OptionsList& optname, const ParameterValue & new_pval)
{
	auto it = parameters_.find(optname);
	if (it != parameters_.end())
		it->second = new_pval;
	else // attribute not found, add it !
		addAttr(optname, new_pval);
}

void OptionsManager::deleteAttr(const OptionsList& optname)
{
	auto it = parameters_.find(optname);
	if (it != parameters_.end()) // erase if element exists
		parameters_.erase(it);
}

bool OptionsManager::hasAttr(const OptionsList& optname) const
{
	auto reqresult = parameters_.find(optname);
	return (reqresult != parameters_.end());
}

std::optional<ParameterValue> OptionsManager::getAttr(const OptionsList& optname) const
{
	if (hasAttr(optname)) // hasAttr will do the checks needed. If it returns true, the value exists.
		return parameters_.find(optname)->second;
	/*
		Here, it would be great to have a way of knowing if this failed, with a warning or something.
	*/
	return { std::nullopt }; // return nullopt, meaning we did not find the requested value ! :(
}
