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
#include "Type.hpp"
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

			// TYPES
			// Getter for the builtin primitive "int" type
			PrimitiveType* getIntType();

			// Getter for the builtin primitive "float" type
			PrimitiveType* getFloatType();

			// Getter for the builtin primitive "char" type
			PrimitiveType* getCharType();

			// Getter for the builtin primitive "bool" type
			PrimitiveType* getBoolType();

			// Getter for the builtin primitive "string" type
			PrimitiveType* getStringType();

			// Getter for the builtin primitive void type
			// (used for function return types)
			PrimitiveType* getVoidType();

			// Returns an ArrayType for a given type.
			ArrayType* getArrayTypeForType(Type* ty);

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

			// Inits all builtin types
			void initBuiltinTypes();

			// An observing pointer to a ASTUnit owned by the vector below that points to the main unit
			// (= the unit that contains the entry point of this module)
			UnitDecl* mainUnit_ = nullptr;

			// All of the units that makes the current module.
			std::vector<UnitDecl*> units_;

			// Built-in types
			// Theses are all initialized by initBuiltinType
			PrimitiveType* voidTy_  = nullptr;
			PrimitiveType* intTy_   = nullptr;
			PrimitiveType* floatTy_ = nullptr;
			PrimitiveType* boolTy_ = nullptr;
			PrimitiveType* charTy_ = nullptr;
			PrimitiveType* stringTy_ = nullptr;

			// Array types
			// Type -> Type[]
			std::map<Type*, ArrayType*> arrayTypes_;

			// Allocators
			LinearAllocator<> alloc_; // Default allocator for the ASTContext
	};
}