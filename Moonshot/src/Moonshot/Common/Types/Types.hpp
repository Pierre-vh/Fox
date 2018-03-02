////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Types.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file declares types/objects/typedefs specific to the interpreter.
// FVal, var_attr, etc.
// 
// This file also declares various helper function to analyze said types 
////------------------------------------------------------////

#pragma once

#include <variant> // std::variant
#include <string> // std::string
#include <inttypes.h>

// fwd decl
namespace Moonshot::var
{
	struct varRef;
	struct varattr;
}

typedef int64_t IntType;
typedef char32_t CharType;
typedef std::monostate NullType;
typedef std::variant<NullType, IntType, float, CharType, std::string, bool, Moonshot::var::varRef> FVal;

namespace Moonshot
{
	class TypeLimits
	{
		public:
			static constexpr IntType IntType_MAX = (std::numeric_limits<IntType>::max)();
			static constexpr IntType IntType_MIN = (std::numeric_limits<IntType>::min)();
			static constexpr CharType CharType_MAX = (std::numeric_limits<CharType>::max)();
			static constexpr CharType CharType_MIN = (std::numeric_limits<CharType>::min)();
	};
	class Types
	{
		public:
			static constexpr std::size_t InvalidIndex	= (std::numeric_limits<std::size_t>::max)();
			static constexpr std::size_t basic_Null		= 0;
			static constexpr std::size_t basic_Int		= 1;
			static constexpr std::size_t basic_Float	= 2;
			static constexpr std::size_t basic_Char		= 3;
			static constexpr std::size_t basic_String	= 4;
			static constexpr std::size_t basic_Bool		= 5;
			static constexpr std::size_t basic_VarRef	= 6;
	};
	namespace var
	{
		struct varRef
		{
			public:
				varRef(const std::string& vname = "");
				std::string getName() const;
				void setName(const std::string& newname);
				operator bool() const;  // checks validity of reference (if name != "");
			private:
				std::string name_;
		};
		struct varattr // Struct holding a var's attributes
		{
			varattr();
			varattr(const std::string &nm);
			varattr(const std::string &nm, const std::size_t &ty, const bool &isK = false);
			operator bool() const;
			// Variable's attribute
			bool isConst_ = false;
			std::string name_ = "";
			std::size_t type_ = Types::basic_Null;

			varRef createRef() const;

			protected:
				bool wasInit_ = false;
		};
		inline bool operator < (const varattr& lhs, const varattr& rhs)
		{
			return lhs.name_ < rhs.name_; // We don't care about the rest, because you can only use a name once.
		}
		inline bool operator == (const varattr& lhs, const varattr& rhs)
		{
			return	(lhs.name_ == rhs.name_) &&
					(lhs.type_ == rhs.type_);
		}
		inline bool operator != (const varattr& lhs, const varattr& rhs)
		{
			return !(lhs == rhs);
		}
	}
}