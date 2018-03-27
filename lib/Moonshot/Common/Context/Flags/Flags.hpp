////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Flags.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This class declares a struct called "Flags", which stores individual flags using
// bitfields, generated by the .def files in this directory.
// The struct class has child structs used to store more specific flags, Common flags, Fox flags, Driver flags,etc.
// 
// Theses are not to be confused with driver flags, even if they're somewhat related to them. In the end, I expect to have 
// a wide variety of flags used to change the interpreter's behaviour, but they won't always have a driver flag (or command line arg) counterpart.
// 
// This allows me to pack a lot of flags within a small space, and still have them
// easily accessible and usable.
////------------------------------------------------------////

namespace Moonshot
{
	// Fox-specific flags.
	struct FoxFlags
	{
		public:
			FoxFlags() = default;
			#define FLAG(FLAG_NAME,FLAG_BASE_VAL) bool FLAG_NAME = FLAG_BASE_VAL
			#include "FoxFlags.def"
		private:
			FoxFlags(const FoxFlags&) = delete;
			FoxFlags& operator=(const FoxFlags&) = delete;
	};
	// Common flags
	struct CommonFlags
	{
		public:
			CommonFlags() = default;
			#define FLAG(FLAG_NAME,FLAG_BASE_VAL) bool FLAG_NAME = FLAG_BASE_VAL
			#include "CommonFlags.def"
		private:
			CommonFlags(const CommonFlags&) = delete;
			CommonFlags& operator=(const CommonFlags&) = delete;
	};
	// Struct that holds every flag. 
	struct Flags
	{
		public:
			Flags() = default;
			FoxFlags fox;
			CommonFlags common;
		private:
			Flags(const Flags&) = delete;
			Flags& operator=(const Flags&) = delete;
	};
}