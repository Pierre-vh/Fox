////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTContext.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// The ASTContext is the foundation of the AST. 
// - It owns the AST and manages it's memory through LinearAllocators
// - It owns the identifier table, accessible through identifierTable()
////------------------------------------------------------////

#pragma once


#include <map>
#include <memory>
#include <vector>
#include "Types.hpp"
#include "Decl.hpp"
#include "Identifiers.hpp"
#include "Fox/Common/LinearAllocator.hpp"

namespace fox
{
	class ASTContext
	{
		public:
			ASTContext();

			// Returns a observing pointer to the unit containing the entry point of the module (if there is one)
			UnitDecl* getMainUnit();
			
			// Registers a unit in this ASTContext
			void addUnit(UnitDecl* unit, bool isMainUnit = false);

			Type getIntType();
			Type getFloatType();
			Type getBoolType();
			Type getStringType();
			Type getCharType();
			Type getVoidType();
			Type getErrorType();

			// Returns an ArrayType for a given type.
			Type getArrayTypeForType(Type ty);

			// Returns an LValueType for a given type
			Type getLValueTypeForType(Type ty);

			// Creates a SemaType
			Type createSemaType(TypeBase *ty = nullptr);

			// ALLOCATOR
			LinearAllocator<>& getAllocator();
			// Resets the ASTContext, freeing the AST and
			// everything allocated within it's allocators.
			void reset();

			// IDENTIFIER TABLE
			IdentifierTable identifiers;

		private:
			// Context shouldn't be copyable.
			ASTContext(const ASTContext&) = delete;
			ASTContext& operator=(const ASTContext&) = delete;

			// Basic types
			Type theIntType_;
			Type theFloatType_;
			Type theCharType_;
			Type theBoolType_;
			Type theStringType_;
			Type theVoidType_;
			Type theErrorType_;

			// An observing pointer to a ASTUnit owned by the vector below that points to the main unit
			// (= the unit that contains the entry point of this module)
			UnitDecl* mainUnit_ = nullptr;

			// All of the units that makes the current module.
			std::vector<UnitDecl*> units_;

			// Array types (Type -> Type[])
			std::map<TypeBase*, ArrayType*> arrayTypes_;

			// LValue types (Type -> @Type)
			std::map<TypeBase*, LValueType*> lvalueTypes_;

			// Allocators
			LinearAllocator<> allocator_; // Default allocator for the ASTContext
	};
}