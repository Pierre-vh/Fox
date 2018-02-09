////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ParameterValue.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// ParameterValue is a wrapper around a variant that provides little helper functions
// to help OptionsManager store Parameter values of different values
////------------------------------------------------------////

#pragma once

#include <variant>		// std::variant

#include "../../Utils/Utils.h"
// ParameterValue could be a wrapper around a std::variant
// Having get<T> methods that returns the desired value, or a default one if the variant
// isn't in the correct state.

// Currently supported arguments value types
	// int
	// bool -> you can use strings in .set and if the string is true/True or false/False it'll convert it to bool for you
// Only numeric type should be implemented here.
// To store strings, use "blank" attributes (set an attribute without value and use hasAttr())

namespace Moonshot
{
	typedef std::variant<std::monostate, int, bool> ParamVal_Variant;
	class ParameterValue
	{
		public:

			ParameterValue()
			{
				rawval_ = std::monostate();
			}

			template<typename T>
			ParameterValue(const T& val)
			{
				set(val); // init with desired value
			}

			~ParameterValue()
			{

			}

			template<typename RTYPE>
			RTYPE get() const
			{
				if (std::holds_alternative<RTYPE>(rawval_))
					return std::get<RTYPE>(rawval_);
				else if constexpr (std::is_arithmetic<RTYPE>())
				{
					RTYPE result;
					std::visit([&](auto& arg){
						result = castHelp<RTYPE>(arg);
					}, rawval_);
					return result;
				}
				// return default
				return RTYPE();
			}

			template<typename T>
			bool holdsType() const
			{
				return std::holds_alternative<T>(rawval_); 
			}
			
			template<typename T>
			void set(const T& val)
			{
				if constexpr(std::is_same<std::string, T>::value)
				{
					if ((val == "true") || (val == "True"))
						rawval_ = true;
					else if ((val == "false") || (val == "False"))
						rawval_ = false;
				}
				rawval_ = val;
				/*
					Error guide :
						If your compiler points to the line of code above this comment for an error, you might have :
							1- Used options.getAttr(..).value_or(T).get<Y>() where T was a type not supported by this class! Check the typedef of 
								the variant at line ~29 (above class) to see which types are available.
				*/
			}

			// Comparison operators
			bool operator ==(const ParameterValue &b) const
			{
				return rawval_ == b.rawval_;
			}
			bool operator !=(const ParameterValue &b) const
			{
				return !(operator==(b)); // uses == to compare
			}

		protected:
			ParamVal_Variant rawval_;

		private:

			template<typename DESIRED,typename VTYPE>
			DESIRED castHelp(const VTYPE& vcont)
			{
				if constexpr (std::is_same<std::monostate,VTYPE>::value)
					return DESIRED();
				else 
					return DESIRED(vcont);
			}
	};
}


