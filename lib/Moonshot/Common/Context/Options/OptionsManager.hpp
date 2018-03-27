////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : OptionsManager.hpp											
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

#include "ParameterValue.hpp"
#include "OptionsList.hpp"

#include <optional> // std::optional
#include <map>


namespace Moonshot
{
	class OptionsManager
	{
		public:
			OptionsManager() = default;

			void addAttr(const OptionsList& optname, const ParameterValue& new_pval = ParameterValue());
			void setAttr(const OptionsList& optname, const ParameterValue& new_pval);

			void deleteAttr(const OptionsList& optname);

			bool hasAttr(const OptionsList& optname) const;

			std::optional<ParameterValue> getAttr(const OptionsList& optname) const; // getAttr returns an optional ParameterValue. when using it, call getAttr().value_or(false).get<...>(); to fall back on something if needed

		private:
			std::map<
				OptionsList,
				ParameterValue
			>	parameters_;
	};
}

