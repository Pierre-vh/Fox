////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : TypeIndex.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file declares types/objects/typedefs specific to the interpreter.
// FoxValue, var_attr, etc.
// 
// This file also declares various helper function to analyze said types 
////------------------------------------------------------////

#pragma once

#include <variant> // std::variant
#include <string> // std::string
#include <inttypes.h>

// fwd decl
namespace Moonshot
{
	struct FoxVariableAttr;
}

typedef int64_t IntType;
typedef char32_t CharType;
typedef float FloatType;
typedef std::monostate VoidType;
typedef std::variant<VoidType, IntType, float, CharType, std::string, bool> FoxValue; // The FoxValue, or FVal for short.

namespace Moonshot
{
	namespace TypeLimits
	{
		static constexpr IntType IntType_MAX = (std::numeric_limits<IntType>::max)();
		static constexpr IntType IntType_MIN = (std::numeric_limits<IntType>::min)();
		static constexpr CharType CharType_MAX = (std::numeric_limits<CharType>::max)();
		static constexpr CharType CharType_MIN = (std::numeric_limits<CharType>::min)();
	};
	namespace TypeIndex
	{
		static constexpr std::size_t InvalidIndex	= (std::numeric_limits<std::size_t>::max)();
		static constexpr std::size_t Void_Type		= 0;
		static constexpr std::size_t basic_Int		= 1;
		static constexpr std::size_t basic_Float	= 2;
		static constexpr std::size_t basic_Char		= 3;
		static constexpr std::size_t basic_String	= 4;
		static constexpr std::size_t basic_Bool		= 5;
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
	struct FoxVariableAttr // Struct holding a var's attributes
	{
		public:
			FoxVariableAttr();
			FoxVariableAttr(const std::string &nm);
			FoxVariableAttr(const std::string &nm, const FoxType &ty);
			operator bool() const;

			std::string getName() const;
			FoxType getType() const;

			void setName(const std::string& str);
			void setType(const FoxType& type);

			std::string dump() const;
		protected:
			std::string name_ = "";
			FoxType type_ = TypeIndex::Void_Type;
			bool wasInit_ = false;
	};
	inline bool operator < (const FoxVariableAttr& lhs, const FoxVariableAttr& rhs)
	{
		return lhs.getName() < rhs.getName();
	}
}