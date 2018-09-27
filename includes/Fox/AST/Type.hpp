////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Type.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file contains the Type & TypeLoc classes
////------------------------------------------------------////

#pragma once

#include "Fox/Common/Source.hpp"
#include "Fox/Common/LLVM.hpp"

namespace fox
{
	class TypeBase;
	class TypeLoc;
	// Type class, an observing pointer to a TypeBase*
	// Used to facilitate passing TypeBase pointers as reference, (Type& instead of TypeBase*&)
	// as well as adding flexibility in case I'd like to add Sugared types one day.
	class Type
	{
		TypeBase* ty_ = nullptr;
		public:
			Type(TypeBase* ty = nullptr);

			TypeBase* getPtr();
			const TypeBase* getPtr() const;

			bool isNull() const;

			TypeBase* operator->();
			const TypeBase* operator->() const;

			explicit operator bool() const;

			// uses dyn_cast_or_null to return the type pointer
			template<typename Ty>
			Ty* getAs()
			{
				return dyn_cast_or_null<Ty>(ty_);
			}

			// uses dyn_cast_or_null to return the type pointer
			template<typename Ty>
			const Ty* getAs() const
			{
				return dyn_cast_or_null<Ty>(ty_);
			}

			// calls isa on the pointer. Returns false
			// if the pointer is null.
			template<typename Ty>
			bool is() const
			{
				return ty_ ? isa<Ty>(ty_) : false;
			}

			bool operator==(const Type& type) const;
			bool operator!=(const Type& type) const;

		private:
			// Forbid TypeLoc->Type conversion
			Type(const TypeLoc&) = delete;
			Type& operator=(const TypeLoc&) = delete;
	};

	// A Type with it's SourceRange, which is used to represent "real" types written down
	// by the user.
	class TypeLoc : public Type
	{
		SourceRange range_;
		public:
			TypeLoc(TypeBase* ty = nullptr, SourceRange range = SourceRange());
			TypeLoc(Type ty, SourceRange range = SourceRange());

			SourceRange getRange() const;

			Type withoutLoc();
			const Type withoutLoc() const; 
		private:
			// For now, disable TypeLoc comparison. We don't need it.
			// Might add a "strict_compare" function tho.
			bool operator==(const Type& type) const = delete;
			bool operator!=(const Type& type) const = delete;

			// Forbid Type->TypeLoc conversion
			TypeLoc(const Type&) = delete;
			TypeLoc& operator=(const Type&) = delete;
	};
}