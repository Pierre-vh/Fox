//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : ASTContext.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/AST/ASTContext.hpp"
#include "Fox/AST/Decl.hpp"
#include "Fox/AST/Types.hpp"
#include "Fox/Common/Builtins.hpp"
#include <cstring>

using namespace fox;

//----------------------------------------------------------------------------//
// getFoxTypeOfFunc impl
//----------------------------------------------------------------------------//

namespace {
  using FnTyParam = FunctionTypeParam;

  template<typename Ty>
  struct TypeConverter {
    static Type get(ASTContext&) {
      static_assert(false, "This type is not supported by the Fox FFI!");
    }
  };
  
  template<typename ... Args> 
  struct ParamConverter {
    static void 
    add(ASTContext&, SmallVectorImpl<FnTyParam>&) {}
  };

  template<typename First, typename ... Args> 
  struct ParamConverter<First, Args...> {
    template<bool ignored = BuiltinArgTypeTrait<First>::ignored>
    static void 
    add(ASTContext& ctxt, SmallVectorImpl<FnTyParam>& params) {
      params.emplace_back(TypeConverter<First>::get(ctxt), /*isMut*/ false);
      ParamConverter<Args...>::add(ctxt, params);
    }

    template<>
    static void 
    add<true>(ASTContext& ctxt, SmallVectorImpl<FnTyParam>& params) {
      ParamConverter<Args...>::add(ctxt, params);
    }
  };

  #define TYPE_CONVERSION(TYPE, GET_IMPL) template<> struct TypeConverter<TYPE>\
    { static Type get(ASTContext& ctxt) { GET_IMPL; } }
  TYPE_CONVERSION(void,       return PrimitiveType::getVoid(ctxt));
  TYPE_CONVERSION(FoxInt,     return PrimitiveType::getInt(ctxt));
  TYPE_CONVERSION(FoxDouble,  return PrimitiveType::getDouble(ctxt));
  TYPE_CONVERSION(bool,       return PrimitiveType::getBool(ctxt));
  TYPE_CONVERSION(FoxChar,    return PrimitiveType::getChar(ctxt));
  #undef TYPE_CONVERSION

  template<typename Rtr, typename ... Args>
  Type getFoxTypeOfFunc(ASTContext& ctxt, Rtr(*)(Args...)) {
    Type returnType = TypeConverter<Rtr>::get(ctxt);
    SmallVector<FnTyParam, 4> params;
    ParamConverter<Args...>::add(ctxt, params);
    return FunctionType::get(ctxt, params, returnType);
  }
}

//----------------------------------------------------------------------------//
// ASTContext
//----------------------------------------------------------------------------//

ASTContext::ASTContext(SourceManager& srcMgr, DiagnosticEngine& diags):
  sourceMgr(srcMgr), diagEngine(diags) {}

ASTContext::~ASTContext() {
  reset();
}

LLVM_ATTRIBUTE_RETURNS_NONNULL LLVM_ATTRIBUTE_RETURNS_NOALIAS
void* ASTContext::allocate(std::size_t size, unsigned align) {
  void* mem = permaAllocator_.allocate(size, align);
  assert(mem && "the allocator returned null memory");
  return mem;
}

void ASTContext::dumpAllocator() const {
  return permaAllocator_.dump();
}

void ASTContext::reset() {
  // Clear sets/maps
  arrayTypes_.clear();
  lvalueTypes_.clear();
  functionTypes_.clear();
  idents_.clear();

  // Clear type singletons
  theIntType_ = nullptr;
  theFloatType_ = nullptr;
  theCharType_ = nullptr;
  theBoolType_ = nullptr;
  theStringType_ = nullptr;
  theVoidType_ = nullptr;
  theErrorType_ = nullptr;

  // Call the cleanups methods
  callCleanups();

  // Reset the allocator, freeing it's memory.
  permaAllocator_.reset();
}

void ASTContext::lookupBuiltin(Identifier id, 
                               SmallVectorImpl<BuiltinFuncDecl*>& results) {
  /// FIXME: This could be greatly improved I think.
  /// Especially since this is going to be called fairly often
  /// (at nearly every lookup)
  /// Of course, back this up by measurements before optimizing anything.
  #define BUILTIN(FUNC, FOX) if(id.getStr() == #FOX)\
    results.push_back(BuiltinFuncDecl::get(*this, BuiltinID::FUNC));
  #include "Fox/Common/Builtins.def"
}

Type ASTContext::getTypeOfBuiltin(BuiltinID id) {
  switch (id) {
    #define BUILTIN(FUNC, FOX)\
      case BuiltinID::FUNC:\
        return getFoxTypeOfFunc(*this, builtin::FUNC);
    #include "Fox/Common/Builtins.def"
    default:
      fox_unreachable("unknown BuiltinID");
  }
}

Identifier ASTContext::getIdentifier(string_view str) {
	// Search the identifiers set (this will use the hash
  // of the string, not its pointer)
	auto it = idents_.find(str);
  // Found, return.
  if(it != idents_.end())
    return Identifier(it->data());
  // The entry doesn't exist yet, so allocate a copy of 
  // the string in the ASTContext and insert it in the set.
  str = allocateCopy(str);
  idents_.insert(str);
  // Return an identifier object using a pointer to that
  // allocated string.
  return Identifier(str.data());
}

Identifier ASTContext::getIdentifier(BuiltinID id) {
  switch (id) {
    #define BUILTIN(FUNC, FOX) case BuiltinID::FUNC:\
      return getIdentifier(#FOX);
    #include "Fox/Common/Builtins.def"
    default:
      fox_unreachable("unknown BuiltinID");
  }
}

string_view ASTContext::allocateCopy(string_view str) {
  std::size_t size = str.size();
  assert(size > 0 && "string is empty");
  const char* buffer = str.data();
  // Allocate a block of the size of the string + 1, so we can add
  // a null terminator.
  char* mem = static_cast<char*>(permaAllocator_.allocate(size+1));
  std::memcpy(mem, buffer, size);
  // Add a null terminator
  static_cast<char*>(mem)[str.size()] = 0;
  return string_view(static_cast<char*>(mem), size);
}

void ASTContext::addCleanup(std::function<void(void)> fn) {
  cleanups_.push_back(fn);
}

void ASTContext::callCleanups() {
  for(const auto& cleanup : cleanups_)
    cleanup();
  cleanups_.clear();
}