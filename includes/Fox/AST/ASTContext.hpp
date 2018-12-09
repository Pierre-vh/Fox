//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : ASTContext.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// The ASTContext is the core of the AST. It contains numerous things, including
// the type singletons, allocators for the AST, and more!
//
// TODO: Remove the "Module" and "Units" things from the ASTContext. Create
//			 specific "Module" and "SourceFile" classes for that (like SwiftC does)
//----------------------------------------------------------------------------//

#pragma once


#include <map>
#include <vector>
#include <set>
#include "Types.hpp"
#include "Decl.hpp"
#include "Identifier.hpp"
#include "Fox/Common/LinearAllocator.hpp"

namespace fox {
	class DiagnosticEngine;
	class SourceManager;
  class ASTContext {
    public:
      ASTContext();

      // Returns a observing pointer to the unit containing the entry point of 
			// the module (if there is one)
      UnitDecl* getMainUnit() const;
      
      // Registers a unit in this ASTContext
      void addUnit(UnitDecl* unit, bool isMainUnit = false);

      // ALLOCATOR
      LinearAllocator<>& getAllocator();
      LinearAllocator<>& getSemaAllocator();

      // Resets the ASTContext, freeing the AST and
      // everything allocated within it's allocators.
      void reset();

      // Frees the memory allocated for the Constraint.
      void freeCS();

			// Returns the unique, ASTContext-owned version of the identifier "str"
			Identifier getIdentifier(const std::string& str);

    protected:
      friend class ArrayType;
      friend class LValueType;
      friend class ErrorType;
      friend class PrimitiveType;

      // Map of Array types (Type -> Type[]) 
      // (managed by ArrayType::get)
      std::map<Type, ArrayType*> arrayTypes;

      // LValue types (Type -> @Type)
      // (managed by LValueType::get)
      std::map<Type, LValueType*> lvalueTypes;


      // Singleton/unique types. Lazily
      // created by their respective classes.
      ErrorType* theErrorType = nullptr;
      PrimitiveType* theIntType = nullptr;
      PrimitiveType* theFloatType = nullptr;
      PrimitiveType* theCharType = nullptr;
      PrimitiveType* theBoolType = nullptr;
      PrimitiveType* theStringType = nullptr;
      PrimitiveType* theVoidType = nullptr;

    private:
      // The ASTContext shouldn't be copyable.
      ASTContext(const ASTContext&) = delete;
      ASTContext& operator=(const ASTContext&) = delete;

      // An observing pointer to a ASTUnit owned by the vector below that points to the main unit
      // (= the unit that contains the entry point of this module)
      UnitDecl* mainUnit_ = nullptr;

      // All of the units that makes the current module.
      std::vector<UnitDecl*> units_;

			// The unique identifiers strings set
			std::set<std::string> idents_;

      // Allocators
      LinearAllocator<> allocator_; // Default allocator
      LinearAllocator<> semaAllocator_; // Sema allocator
  };
}
