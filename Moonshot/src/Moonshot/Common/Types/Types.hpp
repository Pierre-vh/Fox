////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : TypeIndex.hpp											
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
	namespace TypeLimits
	{
		static constexpr IntType IntType_MAX = (std::numeric_limits<IntType>::max)();
		static constexpr IntType IntType_MIN = (std::numeric_limits<IntType>::min)();
		static constexpr CharType CharType_MAX = (std::numeric_limits<CharType>::max)();
		static constexpr CharType CharType_MIN = (std::numeric_limits<CharType>::min)();
	};
	/*
		Notes:
		Basic = a basic type that is available to the users.
		Builtin = a builtin type, that is available to the compiler to use, but not necessarily to the user.
		Strictly speaking ,every type within the compiler is builtin, but 

		All basic types are builtin.
		Not all builtin types are basic.
	*/
	namespace TypeIndex
	{
		static constexpr std::size_t InvalidIndex	= (std::numeric_limits<std::size_t>::max)();
		static constexpr std::size_t Null_Type		= 0;
		static constexpr std::size_t basic_Int		= 1;
		static constexpr std::size_t basic_Float	= 2;
		static constexpr std::size_t basic_Char		= 3;
		static constexpr std::size_t basic_String	= 4;
		static constexpr std::size_t basic_Bool		= 5;
		static constexpr std::size_t VarRef	= 6;
	};
	// The base class you should use to carry type information around.
	class FoxType
	{
		public:
			FoxType() = default;
			FoxType(const std::size_t &basicIndex,const bool &isConstant = false);

			bool isBasic() const;
			bool isArithmetic() const;
			bool isConst() const;
			bool is(const std::size_t& basicindex) const;

			void setType(const std::size_t& basicIndex);
			void setConstAttribute(const bool& val);

			std::size_t getTypeIndex() const;

			std::string getTypeName() const; // returns the name of the type.

			bool compareWith_permissive(const FoxType& other) const;	// Checks if the basic index is the same (types are compatible) without caring about the const flag.
			bool compareWith_strict(const FoxType& other) const;		// Checks like permissive, but checks the const flag too.

			// operators
			FoxType& operator=(const std::size_t& basicIndex);
			
			bool operator==(const std::size_t& basicIndex) const;
			bool operator==(const FoxType& other) const;

			bool operator!=(const std::size_t& basicIndex) const;
			bool operator!=(const FoxType& other) const;
		private:
			bool isconst_;
			std::size_t type_index_ = TypeIndex::InvalidIndex;
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
			varattr(const std::string &nm, const FoxType &ty);
			operator bool() const;
			// Variable's attribute
			std::string name_ = "";
			FoxType type_ = TypeIndex::Null_Type;

			varRef createRef() const;

			std::string dump() const;
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