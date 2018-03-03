////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : FunctionSignature.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// A Struct that represents a function's signature : return type, number,type and name of arguments.		
////------------------------------------------------------////

#pragma once
#include "Moonshot/Common/Types/Types.hpp"
#include <vector>
namespace Moonshot
{
	namespace fn
	{
		// Store a arg's attribute : name, if it's a reference, if it's a const, and it's type.
		struct argattr : public var::varattr
		{
		public:
			argattr(const std::string &nm, const std::size_t &ty, const bool isK, const bool& isref);

			bool isRef_;
			operator bool() const;
		private:
			using varattr::wasInit_;
		};

		inline bool operator==(const argattr& lhs, const argattr& rhs)
		{
			// 2 args are considered identical if their type and name match. Even if one of them is const and the other isn't, they would
			return (lhs.name_ == rhs.name_) && (lhs.type_ == rhs.type_);
		}

		inline bool operator!=(const argattr& lhs, const argattr& rhs)
		{
			return !(lhs == rhs);
		}
		// function signature 
		struct FunctionSignature
		{
			FunctionSignature() = default;
			operator bool() const;

			std::size_t returnType_;
			std::string name_;
			std::vector<argattr> args_;	// info about each arg:
			/*
				use args_.size() to get the number of args
				use args[n] to get the argattr of the n-th arg.
				argattr has different attributes : name,isConst,isRef,type (represented by an index in the FVal)
			*/
		};
	}
}