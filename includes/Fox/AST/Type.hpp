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

namespace fox
{
	class TypeBase;

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
	};
}