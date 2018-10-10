////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Types.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file contains the TypeBase hierarchy.
//
//	Unlike Stmt/Decl/Expr hierarchies, this hierarchy is exclusively
//	allocated/created through static member functions. This is done because
//  most types are unique.
//
////------------------------------------------------------////

#pragma once

#include <string>
#include <cstdint>
#include <list>
#include "llvm/ADT/PointerIntPair.h"
#include "Constraints.hpp"
#include "ASTAligns.hpp"

namespace fox
{
	// Kinds of Types
	enum class TypeKind : std::uint8_t
	{
		#define TYPE(ID,PARENT) ID,
		#define TYPE_RANGE(ID,FIRST,LAST) First_##ID = FIRST, Last_##ID = LAST,
		#include "TypeNodes.def"
	};

	// Forward Declarations
	class ASTContext;

	// TypeBase
	//		Common base for types
	class alignas(align::TypeBaseAlignement) TypeBase
	{
		public:
			/* Returns the type's name in a user friendly form, e.g. "int", "string" */
			std::string toString() const;

			/* Returns the type's name in a developer-friendly form, e.g. "Array(int)" instead of [int]" */
			std::string toDebugString() const;

			TypeKind getKind() const;

			// Returns the element type if this is an ArrayType, or nullptr.
			const TypeBase* unwrapIfArray() const;
			TypeBase* unwrapIfArray();

			// Returns the element type if this is an LValueType, or nullptr.
			const TypeBase* unwrapIfLValue() const;
			TypeBase* unwrapIfLValue();

			// If this type is an LValue, returns it's element type, else
			// returns this.
			const TypeBase* ignoreLValue() const;
			TypeBase* ignoreLValue();

		protected:
			TypeBase(TypeKind tc);

			// Prohibit the use of builtin placement new & delete
			void *operator new(std::size_t) throw() = delete;
			void operator delete(void *) throw() = delete;
			void* operator new(std::size_t, void*) = delete;

			// Only allow allocation through the ASTContext
			// This operator is "protected" so only the ASTContext can create types.
			void* operator new(std::size_t sz, ASTContext &ctxt, std::uint8_t align = alignof(TypeBase));

			// Companion operator delete to silence C4291 on MSVC
			void operator delete(void*, ASTContext&, std::uint8_t) {}
		
		private:
			const TypeKind kind_;
	};

	// BuiltinType
	//		Common base for Built-in types
	class BuiltinType : public TypeBase
	{
		public:
			static bool classof(const TypeBase* type)
			{
				return ((type->getKind() >= TypeKind::First_BuiltinType) 
					&& (type->getKind() <= TypeKind::Last_BuiltinType));
			}

		protected:
			BuiltinType(TypeKind tc);
	};

	// PrimitiveType 
	//		A primitive type (void/int/float/char/bool/string)
	class PrimitiveType : public BuiltinType
	{
		public:
			enum class Kind : std::uint8_t
			{
				VoidTy,
				IntTy,
				FloatTy,
				CharTy,
				StringTy,
				BoolTy
			};

			static PrimitiveType* getString(ASTContext& ctxt);
			static PrimitiveType* getChar(ASTContext& ctxt);
			static PrimitiveType* getFloat(ASTContext& ctxt);
			static PrimitiveType* getBool(ASTContext& ctxt);
			static PrimitiveType* getInt(ASTContext& ctxt);
			static PrimitiveType* getVoid(ASTContext& ctxt);

			Kind getPrimitiveKind() const;

			bool isString() const;
			bool isChar() const;
			bool isFloat() const;
			bool isBool() const;
			bool isInt() const;
			bool isVoid() const;

			static bool classof(const TypeBase* type)
			{
				return (type->getKind() == TypeKind::PrimitiveType);
			}

		private:
			PrimitiveType(Kind kd);

			const Kind builtinKind_;
	};

	// ArrayType
	//		An array of a certain type (can be any type, 
	//		even another ArrayType)
	class ArrayType : public BuiltinType
	{
		public:
			// Returns the UNIQUE ArrayType instance for the given
			// type ty.
			static ArrayType* get(ASTContext& ctxt, TypeBase* ty);

			TypeBase* getElementType();
			const TypeBase* getElementType() const;

			static bool classof(const TypeBase* type)
			{
				return (type->getKind() == TypeKind::ArrayType);
			}

		private:
			// Private because only called by ::get
			ArrayType(TypeBase* elemTy);

			TypeBase* elementTy_= nullptr;
	};

	// LValueType
	//		C/C++-like LValue. e.g. This type is the one
	//		of a DeclRef when the declaration it refers to
	//		is not const.
	class LValueType : public TypeBase
	{
		public:
			// Returns the UNIQUE LValueType instance for the given type "ty"
			static LValueType* get(ASTContext& ctxt, TypeBase* ty);

			TypeBase* getType();
			const TypeBase* getType() const;

			static bool classof(const TypeBase* type)
			{
				return (type->getKind() == TypeKind::LValueType);
			}

		private:
			// Private because only used by ::get
			LValueType(TypeBase* type);

			TypeBase* ty_ = nullptr;
	};

	// ErrorType
	//		A type used to represent that a expression's type
	//		cannot be determined because of an error.
	class ErrorType : public TypeBase
	{
		public:
			// Gets the unique ErrorType instance for the current context.
			static ErrorType* get(ASTContext& ctxt);
			
			static bool classof(const TypeBase* type)
			{
				return (type->getKind() == TypeKind::ErrorType);
			}

		private:
			ErrorType();
	};

	// ConstrainedType
	//		A type which couldn't be "decided" and is waiting to
	//		be unified. The proposed type which will "substitute" 
	//		this ConstrainedType MUST satisfy every constraint.
	//
	//		This class contains a vector of constraints, a pointer
	//		to a potential substitution and a "up to date" marker for
	//		the substitution. The latter is set to false if we add another
	//		constraint, and set back to true if we call setSubstitution().
	//		Outdated ConstraintedTypes should be checked again to ensure the
	//		substitution is still okay.
	//
	//		Theses types are allocated in the ConstraintAllocator of the ASTContext,
	//		like Constraints themselves.
	//
	//		As opposed to other types, this type is not unique, and
	//		one is generated for every expression that needs one. 
	//		On the other hand, the constraints are immutable and you can't
	//		remove a constraint once it's added to this type.
	//
	//		Example:
	//			Empty array literals '[]' generate a ConstrainedType 
	//			containing a single "ArrayCS"
	class ConstrainedType : public TypeBase
	{
		public:
			// Creates a new instance of the ConstrainedType class
			static ConstrainedType* create(ASTContext& ctxt);

			TypeBase* getSubstitution();
			const TypeBase* getSubstitution() const;

			// Returns true if the type has a substitution
			// (type isn't null)
			bool hasSubstitution() const;

			void setSubstitution(TypeBase* subst);

			// Returns true if the substitution is outdated, then it should
			// be checked again. A substitution becomes outdated iff
			// we add another constraint. It's updated
			// when you call setSubstitution.
			// If there is no substitution, returns true.
			bool isSubstitutionOutdated() const;

			void resetSubstitution();

			ConstraintList::iterator cs_begin();
			ConstraintList::const_iterator cs_begin() const;

			ConstraintList::iterator cs_end();
			ConstraintList::const_iterator cs_end() const;

			ConstraintList& getConstraints();

			std::size_t numConstraints() const;
			void addConstraint(Constraint* cs);

			static bool classof(const TypeBase* type)
			{
				return (type->getKind() == TypeKind::ConstrainedType);
			}

		private:
			// Private because only called by ::create
			ConstrainedType();

			// Override the new operator to use the ConstraintAllocator in the
			// ASTContext to allocate ConstrainedTypes.
			void* operator new(std::size_t sz, ASTContext &ctxt, std::uint8_t align = alignof(TypeBase));

			void markAsUpToDate();
			void markAsOutdated();

			ConstraintList constraints_;
			// Pointer + "up to date" marker. If the int is 1,
			// the pointer is up to date, 0 if it isnt.
			llvm::PointerIntPair<TypeBase*, 1> subst_;
	};

}