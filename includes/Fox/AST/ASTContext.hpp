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
#include "Fox/Common/Builtins.hpp"
#include "Fox/Common/LLVM.hpp"
#include "Fox/Common/LinearAllocator.hpp"
#include "Fox/Common/string_view.hpp"
#include "llvm/ADT/SmallVector.h"
#include <unordered_map>
#include <unordered_set>
#include <functional>

namespace fox {
	class DiagnosticEngine;
	class SourceManager;

  /// The ASTContext is pretty much the core of the AST. It owns
  /// the allocators used to allocate AST nodes and keeps track of type singletons.
  ///
  /// The ASTContext also contains a reference to the DiagnosticEngine
  /// and SourceManager instances used by the AST and related classes (Lexer, Parser,
  /// Semantic Analysis)
  class ASTContext {
    public:
      // The ASTContext shouldn't be copyable.
      ASTContext(const ASTContext&) = delete;
      ASTContext& operator=(const ASTContext&) = delete;

      /// Constructor for the ASTContext
      /// \param srcMgr the SourceManager instance that should be used by the
      ///        AST and related classes.
      /// \param diags the DiagnosticEngine instance that should be used by the
      ///        AST and related classes.
      ASTContext(SourceManager& srcMgr, DiagnosticEngine& diags);
      ~ASTContext();

      /// Allocates memory using the default allocator
      /// \param size the size of the memory block to allocate
      /// \param align the alignement of the memory block
      /// \returns a pointer to the allocate memory. Never null unless
      ///          we run out of memory.
      LLVM_ATTRIBUTE_RETURNS_NONNULL LLVM_ATTRIBUTE_RETURNS_NOALIAS
      void* allocate(std::size_t size, unsigned align);

      /// Dumps the ASTContext's allocator to stdout.
      void dumpAllocator() const;

      /// Resets the ASTContext, calling cleanups and freeing the AST.
      void reset();

			/// \param str an identifier string
      /// \returns the unique "Identifier" object for this string.
			Identifier getIdentifier(string_view str);

      /// \param id the builtin id
      /// \returns the unique "Identifier" object for the builtin 
      ///          with id \p id
      Identifier getIdentifier(BuiltinID id);

      /// \param str a string
      /// \returns a NULL-TERMINATED version of this string, allocated 
      ///         in the ASTContext's allocator.
      string_view allocateCopy(string_view str);

      /// Add a cleanup function to be called when the ASTContext's permanent
      /// allocator frees it's memory.
      /// \param fn the cleanup function that should be called when free()
      ///        is called.
      void addCleanup(std::function<void(void)> fn);

      /// Templated version of \ref "addCleanup" that calls an object's destructor.
      template<typename Ty>
      void addDestructorCleanup(Ty& obj) {
        addCleanup([&obj]() {
          obj.~Ty();
        });
      }
      
      /// The SourceManager instance tied to the ASTContext.
      SourceManager& sourceMgr;

      /// The DiagnosticEngine instance tied to the ASTContext.
      DiagnosticEngine& diagEngine;

    private:
      friend class ArrayType;
      friend class LValueType;
      friend class ErrorType;
      friend class PrimitiveType;
      friend class FunctionType;

      /// Calls the cleanup functions and resets the "cleanups" vector.
      void callCleanups();

      /// The stored cleanup functions
      SmallVector<std::function<void(void)>, 4> cleanups_;

      /// Map of Array types (maps a Type to a Array(Type)), used by the
      /// \ref ArrayType class.
      std::unordered_map<TypeBase*, ArrayType*> arrayTypes_;

      /// LValue types (maps a Type to a LValue(Type)), used by the \ref LValueType
      /// class.
      std::unordered_map<TypeBase*, LValueType*> lvalueTypes_;

      /// Function types (maps a signature of a FunctionType to the unique
      /// instance of the FunctionType).
      /// Managed by the \ref FunctionType class.
      std::unordered_map<std::size_t, FunctionType*> functionTypes_;

      // Singleton/unique types. Lazily created by their respective classes.
      ErrorType* theErrorType_      = nullptr;
      PrimitiveType* theIntType_    = nullptr;
      PrimitiveType* theFloatType_  = nullptr;
      PrimitiveType* theCharType_   = nullptr;
      PrimitiveType* theBoolType_   = nullptr;
      PrimitiveType* theStringType_ = nullptr;
      PrimitiveType* theVoidType_   = nullptr;

      /// The set of unique identifier strings.
      std::unordered_set<string_view> idents_;

      /// The main AST allocator, used to allocate the AST.
      LinearAllocator permaAllocator_; 
  };
}
