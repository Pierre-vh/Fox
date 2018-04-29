////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTContext.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// The ASTContext holds some "contextual" information about the AST, which means:
// - Long-lived AST Nodes, like Types nodes (Currently, only builtin types)
// - The AST's identifier Table, accessible through identifierTable()
// 
// It also "owns" the AST as a whole, and offer some useful function, like getMainUnit(), which returns a pointer to the main
// ASTUnit. (the unit that contains the entry point of the program)
//
// Note that it's very different from the Context and DeclContext classes.
// Context is the "compilation context", which means it holds information about the current compilation task :
//	-> The ASTContext 
//	-> The DiagnosticsEngine
//	-> The FlagsManager
// and the DeclContext is, literally, a "declaration context", which means it registers every declaration it needs to know
// about.
////------------------------------------------------------////

#pragma once


#include <map>

#include "Moonshot/Fox/Common/Memory.hpp"
#include "Decl.hpp"
#include "Type.hpp"
#include "IdentifierTable.hpp"

namespace Moonshot
{
	class ASTContext
	{
		public:
			ASTContext();

			// Returns a observing pointer to the unit containing the entry point of the module (if there is one)
			UnitDecl* getMainUnit();
			
			// Take ownership of the unit, and mark it as the main unit. 
			// Once it took ownership, it returns a observing pointer to that unit.
			UnitDecl* setMainUnit(std::unique_ptr<UnitDecl> unit);

			// Takes ownership of the unit.
			// Once ownership is taken, it returns a observing pointer to that unit.
			UnitDecl* addUnit(std::unique_ptr<UnitDecl> unit);

			const PrimitiveType* getPrimitiveIntType() const;
			const PrimitiveType* getPrimitiveFloatType() const;
			const PrimitiveType* getPrimitiveCharType() const;
			const PrimitiveType* getPrimitiveBoolType() const;
			const PrimitiveType* getPrimitiveStringType() const;
			const PrimitiveType* getPrimitiveVoidType() const;

			// Returns an ArrayType for a given type.
			const ArrayType* getArrayTypeForType(const Type* ty);

			// The Identifier table.
			// It's public because it should be accessible by everyone.
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
				// Theses are all initialized to nullptr, but are properly set by
				// initBuiltinTypes().
			std::unique_ptr<PrimitiveType> primitiveVoidTy_	= nullptr;
			std::unique_ptr<PrimitiveType> primitiveIntTy_	= nullptr;
			std::unique_ptr<PrimitiveType> primitiveFloatTy_= nullptr;
			std::unique_ptr<PrimitiveType> primitiveBoolTy_	= nullptr;
			std::unique_ptr<PrimitiveType> primitiveCharTy_	= nullptr;
			std::unique_ptr<PrimitiveType> primitiveStringTy_ = nullptr;

			// Array types
			std::map<const Type*, std::unique_ptr<ArrayType>> arrayTypes_;
	};
}