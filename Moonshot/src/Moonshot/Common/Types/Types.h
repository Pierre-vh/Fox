////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Types.h											
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
typedef wchar_t CharType;
typedef std::variant<std::monostate, IntType, float, CharType, std::string, bool, Moonshot::var::varRef> FVal;

namespace Moonshot
{
	namespace limits
	{
		static constexpr IntType IntType_MAX = INT64_MAX;
		static constexpr IntType IntType_MIN = INT64_MIN;
		static constexpr CharType CharType_MAX = WCHAR_MAX;
		static constexpr CharType CharType_MIN = WCHAR_MIN;
	}
	namespace fv_util
	{
		// Variables : Indexes
		namespace indexes
		{
			static constexpr std::size_t invalid_index	= (std::numeric_limits<std::size_t>::max)();

			static constexpr std::size_t fval_null		= 0;
			static constexpr std::size_t fval_int		= 1;
			static constexpr std::size_t fval_float		= 2;
			static constexpr std::size_t fval_char		= 3;
			static constexpr std::size_t fval_str		= 4;
			static constexpr std::size_t fval_bool		= 5;
			static constexpr std::size_t fval_varRef	= 6;
		}
	}
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
			bool isConst = false;
			std::string name_ = "";
			std::size_t type_ = fv_util::indexes::fval_null;

			varRef createRef() const;

			private:
				bool wasInit_ = false;
		};
		inline bool operator < (const varattr& lhs, const varattr& rhs)
		{
			return lhs.name_ < rhs.name_; // We don't care about the rest, because you can only use a name once.
		}
		inline bool operator == (const varattr& lhs, const varattr& rhs)
		{
			return (lhs.name_ == rhs.name_) &&
				(lhs.isConst == rhs.isConst) &&
				(lhs.type_ == rhs.type_);
		}
		inline bool operator != (const varattr& lhs, const varattr& rhs)
		{
			return !(lhs == rhs);
		}
	}
}