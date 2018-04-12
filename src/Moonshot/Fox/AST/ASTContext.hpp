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

#include <memory>
#include <vector>

#include "ASTUnit.hpp"
#include "Types.hpp"
#include "IdentifierTable.hpp"

namespace Moonshot
{
	class ASTContext
	{
		public:
			ASTContext();

			// Returns a observing pointer to the unit containing the entry point of the module (if there is one)
			ASTUnit* getMainUnit();
			
			// Take ownership of the unit, and mark it as the main unit. 
			// Once it took ownership, it returns a observing pointer to that unit.
			ASTUnit* setMainUnit(std::unique_ptr<ASTUnit> unit);

			// Takes ownership of the unit.
			// Once ownership is taken, it returns a observing pointer to that unit.
			ASTUnit* addUnit(std::unique_ptr<ASTUnit> unit);

			IdentifierTable& identifierTable();

			TypePtr getBuiltinIntType();
			TypePtr getBuiltinFloatType();
			TypePtr getBuiltinCharType();
			TypePtr getBuiltinBoolType();
			TypePtr getBuiltinStringType();
			TypePtr getBuiltinVoidType();
		private:
			ASTContext(const ASTContext&) = delete;
			ASTContext& operator=(const ASTContext&) = delete;

			// Init all builtin types
			void initBuiltinTypes();

			// An observing pointer to a ASTUnit owned by the vector below that points to the main unit
			// (= the unit that contains the entry point of this module)
			ASTUnit* mainUnit_ = nullptr;

			// All of the units that makes the current program.
			std::vector<std::unique_ptr<ASTUnit>> units_;

			// Identifier table
			IdentifierTable idents_;

			// Built-in types
				// Theses are all initialized to nullptr, but are properly set by
				// initBuiltinTypes().
			TypePtr builtinVoid_	= nullptr;
			TypePtr builtinInt_		= nullptr;
			TypePtr builtinFloat_	= nullptr;
			TypePtr builtinBool_	= nullptr;
			TypePtr builtinChar_	= nullptr;
			TypePtr builtinString_	= nullptr;
	};
}