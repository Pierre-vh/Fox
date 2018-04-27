////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Flags.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This class declares enum for each flag kind, and a FlagManager that groups them all and provide nice and easy access to the flag values.
// 
// Theses flags are not to be confused with command line "flags" (or args), even if they're somewhat related to them. In the end, I expect to have 
// a wide variety of flags used to change the interpreter's behaviour, but they won't always have a command line arg counterpart. Some might just be here
// for tweaking or customizing the interpreter's behaviour.
////------------------------------------------------------////

#pragma once

#include <map>
#include "Moonshot/Fox/Basic/Exceptions.hpp"

namespace Moonshot
{
	enum class FlagID
	{
		#define FLAG(FLAG_NAME,FLAG_BASE_VAL) FLAG_NAME,
		#include "Flags/FlagsAll.def"	
	};
	#define UNKNOWN_KEY_EXCEPTION "Enum value does not exists in map. This can happen if you add a new enum value without using the .def files!"
	class FlagsManager
	{
		public:
			FlagsManager() = default;

			inline bool isSet(const FlagID& fid) const
			{
				if (existsInMap(flags_, fid))
					return flags_.find(fid)->second;
				else
					throw std::out_of_range(UNKNOWN_KEY_EXCEPTION);
			}

			inline void set(const FlagID& fid)
			{
				if (existsInMap(flags_, fid))
					flags_[fid] = true;
				else
					throw std::out_of_range(UNKNOWN_KEY_EXCEPTION);
			}
			
			void unset(const FlagID& fid)
			{
				if (existsInMap(flags_, fid))
					flags_[fid] = false;
				else
					throw std::out_of_range(UNKNOWN_KEY_EXCEPTION);
			}
		private:
			template<typename KEY, typename DATA>
			inline bool existsInMap(const std::map<KEY, DATA> &mmap, const KEY& key) const
			{
				return mmap.find(key) != mmap.end();
			}

			std::map<FlagID, bool> flags_ =
			{
				#define FLAG(FLAG_NAME,FLAG_BASE_VAL) { FlagID::FLAG_NAME, FLAG_BASE_VAL },
				#include "Flags/FlagsAll.def"	
		
			};
	};
	#undef UNKNOWN_KEY_EXCEPTION
}