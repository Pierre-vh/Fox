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
#include "Fox/Common/BuiltinID.hpp"
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

      /// Lookup function for builtins: places a pointer to
      /// every builtin with the Identifier \p id in \p results
      void lookupBuiltin(Identifier id, 
                         SmallVectorImpl<BuiltinFuncDecl*>& results);

      /// \returns the FunctionType of the public builtin function with ID \p id
      ///          for this ASTContext.
      Type getPublicBuiltinFuncType(BuiltinID id);

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

      /// \returns the identifier of the entry point function.
      Identifier getEntryPointIdentifier() const;

      /// \returns the type of the entry point function
      Type getEntryPointType() const;

      /// \returns the entry point function, nullptr if there isn't any.
      FuncDecl* getEntryPoint() const;
      /// sets the entry point to \p decl. Once an entry point has been set, it
      /// cannot be changed anymore.
      void setEntryPoint(FuncDecl* decl);

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
      // Friend every Type class
      // FIXME: Once I do PImpl on the ASTContext, remove these and implement
      // every Type::create/get method in ASTContext.cpp instead.
      #define TYPE(TYPE, PARENT) friend TYPE;
      #include "TypeNodes.def"
      friend BuiltinFuncDecl;

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

      /// The map of BuiltinID -> BuiltinFuncDecl
      std::unordered_map<BuiltinID, BuiltinFuncDecl*> builtinFuncs_;

      // Singleton/unique types. Lazily created by their respective classes.
      ErrorType*   theErrorType_   = nullptr;
      IntegerType* theIntType_     = nullptr;
      DoubleType*  theDoubleType   = nullptr;
      CharType*    theCharType_    = nullptr;
      BoolType*    theBoolType_    = nullptr;
      StringType*  theStringType_  = nullptr;
      VoidType*    theVoidType_    = nullptr;

      /// The entry point of the program
      FuncDecl* entryPoint_ = nullptr;

      /// The identifier of the entry point function ('main').
      Identifier entryPointIdentifier_;

      /// The type of the entry point function ('() -> void')
      FunctionType* entryPointType_ = nullptr;

      /// The set of unique identifier strings.
      std::unordered_set<string_view> idents_;

      /// The main AST allocator, used to allocate the AST.
      LinearAllocator permaAllocator_; 
  };
}
