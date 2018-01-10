////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : OptionsManager.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This class manages the options for the current context. 
// It's essentially a wrapper around a map of options, but it does a bit more!
////------------------------------------------------------////

// TO DO :
// Include an "OptionsManager" class into the context class.
// Options manager will be a wrapper for a std::vector<std::pair<string,string>> (module,attr_name)
// Options manager should have the following methods:
// .setAttr(module,attr_name)
// .hasAttr(module,attr_name)
// .getAttr(module,attr_name) returns ParameterValue
// Options Manager will be a private variable, created in Context's constructor.
// If an attribute is set, the interpreter might change behaviour
// This system could be extended in the future to have a function such as 

#pragma once

#include "ParameterValue.h"
#include <iostream>
#include <map>
#include <string>
#include <tuple> // std::pair

namespace Moonshot
{
	class OptionsManager
	{
		public:
			OptionsManager();
			~OptionsManager();

			void addAttr(const std::string& new_module_name, const std::string& new_attr_name, const ParameterValue& new_pval = ParameterValue());
			void setAttr(const std::string& module_name, const std::string& attr_name, const ParameterValue& new_pval);

			void deleteAttr(const std::string& module_name, const std::string& attr_name);

			bool hasAttr(const std::string& module_name, const std::string& attr_name) const;
			ParameterValue getAttr(const std::string& module_name, const std::string& attr_name) const;

		private:
			std::map<
				std::pair<
				std::string,
				std::string
				>,
				ParameterValue
			>	parameters_;
	};
}

