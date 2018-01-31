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
#include <sstream> // std::stringstream
#include <type_traits> // std::is_same

#include "../../Common/Context/Context.h" // context
#include "../../Common/Exceptions/Exceptions.h"
#include "../../Fox/Lexer/Token.h"
#include "../../Fox/Util/Enums.h"

// fwd decl
namespace Moonshot::var
{
	struct varRef;
	struct varattr;
}

// Alias for a variant holding every type possible in the interpreter.
typedef std::variant<std::monostate,int, float, char, std::string, bool, Moonshot::var::varRef> FVal;

namespace Moonshot
{
	namespace fv_util
	{
		// FValue traits class, to use with templated functions.
		template <typename T>
		struct fval_traits
		{
			constexpr static bool isBasic =
					std::is_same<T,int>::value				||
					std::is_same<T,float>::value			||
					std::is_same<T,char>::value				||
					std::is_same<T,std::string>::value		||
					std::is_same<T,bool>::value
				;
			constexpr static bool isArithmetic = isBasic && !std::is_same<T, std::string>::value;
		
			constexpr static inline bool isEqualTo(const std::size_t& index) // Checks if T represent the same type as index
			{
				if constexpr(std::is_same<T, std::monostate>::value)
					return index == fval_null;
				else if constexpr(std::is_same<T, int>::value)
					return index == fval_int;
				else if constexpr(std::is_same<T, float>::value)
					return index == fval_float;
				else if constexpr(std::is_same<T, char>::value)
					return index == fval_char;
				else if constexpr(std::is_same<T, std::string>::value)
					return index == fval_str;
				else if constexpr(std::is_same<T, bool>::value)
					return index == fval_bool;
				else if constexpr(std::is_same<T, var::varRef>::value)
					return index == fval_varRef;
				else
				{
					throw std::logic_error("Defaulted");
					return false;
				}
			}
		};
		
		// Dump (debugging) functions
		std::string dumpFVal(const FVal &var);
		std::string dumpVAttr(const var::varattr &var);

		// returns a sample fval for an index.
		FVal getSampleFValForIndex(const std::size_t& t);

		// Get a user friendly name for an index.
		std::string indexToTypeName(const std::size_t& t);

		// Index utility function
		bool isBasic(const std::size_t& t); // Is the type a string/bool/char/int/float ?
		bool isArithmetic(const std::size_t& t);
		bool isValue(const std::size_t& t);

		bool canAssign(Context& context_,const std::size_t &lhs, const std::size_t &rhs); // Checks if the lhs and rhs are compatible.
																		// Compatibility : 
																		// Arithmetic type <-> Arithmetic Type = ok
																		// string <-> string = ok
																		// else : error.
		// This function returns true if the type of basetype can be cast to the type of goal.
		// if i want to implement explicit casts from strings to arithmetic type later, this can be done "easily" by adding
		// const bool& isExplicit = false to the signature
		// And, in the function body, add a check if(isExplicit && (basetype==fval_str) && isArithmetic(goal)) return true;
		// For now, i leave it out, but it might be added later!
		bool canCastTo(const std::size_t &goal, const std::size_t &basetype);

		// returns the type of the biggest of the 2 arguments. 
		// Example outputs : 
		// lhs : fval_int
		// rhs : fval_float
		// output : fval_float.
		std::size_t getBiggest(const std::size_t &lhs, const std::size_t &rhs);

		// Variables : Indexes
		// Thanks, I guess ! This looks healthier than using -1 as invalid index. https://stackoverflow.com/a/37126153
		static constexpr std::size_t invalid_index = std::numeric_limits<std::size_t>::max();
		// How to remember values of index
		static constexpr std::size_t fval_null	= 0;
		static constexpr std::size_t fval_int	= 1;
		static constexpr std::size_t fval_float = 2;
		static constexpr std::size_t fval_char	= 3;
		static constexpr std::size_t fval_str	= 4;
		static constexpr std::size_t fval_bool  = 5;
		static constexpr std::size_t fval_varRef = 6;

		// Map for converting type kw to a FVal index.
		const std::map<keywordType, std::size_t> kTypeKwToIndex_dict =
		{
			{ keywordType::T_INT	, fval_int	},
			{ keywordType::T_FLOAT	, fval_float},
			{ keywordType::T_BOOL	, fval_bool },
			{ keywordType::T_STRING , fval_str	},
			{ keywordType::T_CHAR	, fval_char }

		};

		std::size_t typeKWtoSizeT(const keywordType& kw);

		const std::map<std::size_t, std::string> kType_dict =
		{
			{ fval_null				, "NULL" },
			{ fval_int				, "INT" },
			{ fval_float			, "FLOAT" },
			{ fval_char				, "CHAR" },
			{ fval_bool				, "BOOL" },
			{ fval_str				, "STRING" },
			{ fval_varRef			, "VAR_ATTR (ref)"},
			{ invalid_index			, "!INVALID_FVAL!" }
		};
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
			std::string name = "";
			std::size_t type = fv_util::fval_null;

			varRef createRef() const;

		private:
			bool wasInit_ = false;
		};
		inline bool operator < (const varattr& lhs, const varattr& rhs)
		{
			return lhs.name < rhs.name; // We don't care about the rest, because you can only use a name once.
		}
		inline bool operator == (const varattr& lhs, const varattr& rhs)
		{
			return (lhs.name == rhs.name) &&
				(lhs.isConst == rhs.isConst) &&
				(lhs.type == rhs.type);
		}
		inline bool operator != (const varattr& lhs, const varattr& rhs)
		{
			return !(lhs == rhs);
		}
	}
}