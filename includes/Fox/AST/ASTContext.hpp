//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : ASTContext.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// The ASTContext is the core of the AST
// - It owns the AST and manages it's memory through LinearAllocator
// - It owns the identifier table, accessible through identifierTable()
//----------------------------------------------------------------------------//

#pragma once


#include <map>
#include <memory>
#include <vector>
#include "Types.hpp"
#include "Decl.hpp"
#include "Identifiers.hpp"
#include "Fox/Common/LinearAllocator.hpp"

namespace fox {
  class ASTContext {
    public:
      ASTContext();

      // Returns a observing pointer to the unit containing the entry point of the module (if there is one)
      UnitDecl* getMainUnit();
      
      // Registers a unit in this ASTContext
      void addUnit(UnitDecl* unit, bool isMainUnit = false);

      // ALLOCATOR
      LinearAllocator<>& getAllocator();
      LinearAllocator<>& getCSAllocator();

      // Resets the ASTContext, freeing the AST and
      // everything allocated within it's allocators.
      void reset();

      // Frees the memory allocated for the Constraint.
      void freeCS();

      // The identifier table
      IdentifierTable identifiers;
    protected:
      friend class ArrayType;
      friend class LValueType;
      friend class ErrorType;
      friend class PrimitiveType;
      friend class Constraint;

      // Map of Array types (Type -> Type[]) 
      // (managed by ArrayType::get)
      std::map<TypeBase*, ArrayType*> arrayTypes;

      // LValue types (Type -> @Type)
      // (managed by LValueType::get)
      std::map<TypeBase*, LValueType*> lvalueTypes;


      // Singleton/unique types. Lazily
      // created by their respective classes.
      ErrorType* theErrorType = nullptr;
      PrimitiveType* theIntType = nullptr;
      PrimitiveType* theFloatType = nullptr;
      PrimitiveType* theCharType = nullptr;
      PrimitiveType* theBoolType = nullptr;
      PrimitiveType* theStringType = nullptr;
      PrimitiveType* theVoidType = nullptr;

      // As an optimization, trivial
      // constraints are unique, since they're immutable
      // and have no members.
      Constraint* theArrayCS = nullptr;

    private:
      // The ASTContext shouldn't be copyable.
      ASTContext(const ASTContext&) = delete;
      ASTContext& operator=(const ASTContext&) = delete;

      // An observing pointer to a ASTUnit owned by the vector below that points to the main unit
      // (= the unit that contains the entry point of this module)
      UnitDecl* mainUnit_ = nullptr;

      // All of the units that makes the current module.
      std::vector<UnitDecl*> units_;

      // Allocators
      LinearAllocator<> allocator_; // Default allocator
      LinearAllocator<> csAllocator_; // Constraint allocator
  };
}