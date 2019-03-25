//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : ASTContext.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// Contains the ASTContext class
//----------------------------------------------------------------------------//

#pragma once

#include "Identifier.hpp"
#include "ASTFwdDecl.hpp"
#include "Fox/Common/LinearAllocator.hpp"
#include "Fox/Common/string_view.hpp"
#include "Fox/Common/LLVM.hpp"
#include "llvm/ADT/SmallVector.h"
#include <map>
#include <unordered_set>
#include <functional>

namespace fox {
	class DiagnosticEngine;
	class SourceManager;

  // The ASTContext is pretty much the centerpiece of the AST. It owns
  // the allocators used to allocate the AST, keeps track of type singletons,
  // of the UnitDecls, etc.
  //
  // The ASTContext also contains a reference to the "attached" DiagnosticEngine
  // and SourceManagers.
  class ASTContext {
    public:
      // The ASTContext shouldn't be copyable.
      ASTContext(const ASTContext&) = delete;
      ASTContext& operator=(const ASTContext&) = delete;

      // Constructor
      ASTContext(SourceManager& srcMgr, DiagnosticEngine& diags);
      ~ASTContext();

      // Allocates memory using the default allocator
      LLVM_ATTRIBUTE_RETURNS_NONNULL LLVM_ATTRIBUTE_RETURNS_NOALIAS
      void* allocate(std::size_t size, unsigned align);

      void dumpAllocator() const;

      // Resets the ASTContext, freeing the AST and
      // everything allocated within it's allocators.
      void reset();

			// Returns the unique, ASTContext-owned version of the identifier "str"
			Identifier getIdentifier(string_view str);

      // Allocates a copy of "str" inside the main allocator.
      string_view allocateCopy(string_view str);

      // Add a cleanup function to be called when the ASTContext's permanent
      // allocator frees it's memory.
      void addCleanup(std::function<void(void)> fn);

      // Adds a cleanup function that calls the object's destructor.
      template<typename Ty>
      void addDestructorCleanup(Ty& obj) {
        addCleanup([&obj]() {
          obj.~Ty();
        });
      }

      SourceManager& sourceMgr;
      DiagnosticEngine& diagEngine;

    private:
      friend class ArrayType;
      friend class LValueType;
      friend class ErrorType;
      friend class PrimitiveType;
      friend class FunctionType;

      // Calls the cleanup functions reset the "cleanups" vector.
      void callCleanups();

      // Returns a non-const reference to the allocator desired.
      LinearAllocator& getAllocator();

      SmallVector<std::function<void(void)>, 4> cleanups_;

      // Map of Array types (maps a Type to a Type[]) 
      // (managed by ArrayType::get)
      std::map<TypeBase*, ArrayType*> arrayTypes_;

      // LValue types (maps a Type to a @Type)
      // (managed by LValueType::get)
      std::map<TypeBase*, LValueType*> lvalueTypes_;

      // Function types (maps a hash of a FunctionType's data to it's instance)
      // (managed by FunctionType::get)
      std::map<std::size_t, FunctionType*> functionTypes_;

      // Singleton/unique types. Lazily
      // created by their respective classes.
      ErrorType* theErrorType_ = nullptr;
      PrimitiveType* theIntType_ = nullptr;
      PrimitiveType* theFloatType_ = nullptr;
      PrimitiveType* theCharType_ = nullptr;
      PrimitiveType* theBoolType_ = nullptr;
      PrimitiveType* theStringType_ = nullptr;
      PrimitiveType* theVoidType_ = nullptr;

      // The unique identifiers strings set
      std::unordered_set<std::string> idents_;

      // The main AST allocator, used for long lived objects.
      LinearAllocator permaAllocator_; 
  };
}
