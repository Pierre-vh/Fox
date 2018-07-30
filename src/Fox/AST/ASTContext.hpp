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
			
			// Takes ownership of the unit.
			// Once ownership is taken, it returns a observing pointer to that unit.
			// Takes an optional boolean parameter, set it to true so that the unit added is recognized as the main unit
			// in this ASTContext
			UnitDecl* addUnit(std::unique_ptr<UnitDecl> unit, bool isMainUnit = false);

			PrimitiveType* getPrimitiveIntType();
			PrimitiveType* getPrimitiveFloatType();
			PrimitiveType* getPrimitiveCharType();
			PrimitiveType* getPrimitiveBoolType();
			PrimitiveType* getPrimitiveStringType();
			PrimitiveType* getPrimitiveVoidType();

			// Returns an ArrayType for a given type.
			ArrayType* getArrayTypeForType(Type* ty);

			// Getter for the Allocator
			LinearAllocator<>& getAllocator();

			// Identifier table
			IdentifierTable identifiers;
		private:
			// Context shouldn't be copyable.
			ASTContext(const ASTContext&) = delete;
			ASTContext& operator=(const ASTContext&) = delete;

			// Init all builtin types
				// /!\ This must be called prior to using any getType function ! -> This should be called in the Constructor !
			void initBuiltinTypes();

			// An observing pointer to a ASTUnit owned by the vector below that points to the main unit
			// (= the unit that contains the entry point of this module)
			UnitDecl* mainUnit_ = nullptr;

			// All of the units that makes the current module.
			std::vector<std::unique_ptr<UnitDecl>> units_;

			// Built-in types
			// Theses are all initialized by initBuiltinType
			std::unique_ptr<PrimitiveType> primitiveVoidTy_;
			std::unique_ptr<PrimitiveType> primitiveIntTy_;
			std::unique_ptr<PrimitiveType> primitiveFloatTy_;
			std::unique_ptr<PrimitiveType> primitiveBoolTy_;
			std::unique_ptr<PrimitiveType> primitiveCharTy_;
			std::unique_ptr<PrimitiveType> primitiveStringTy_;

			// Array types
			std::map<Type*, std::unique_ptr<ArrayType>> arrayTypes_;

			// Allocators
			LinearAllocator<> alloc_;
	};
}