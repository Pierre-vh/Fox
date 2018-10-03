////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Types.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file contains the TypeBase hierarchy.
////------------------------------------------------------////

#pragma once

#include <string>
#include <cstdint>
#include <list>
#include "llvm/ADT/PointerIntPair.h"
#include "Constraints.hpp"

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
	class TypeBase
	{
		public:
			/* Should return the type's name in a user friendly form, e.g. "int", "string" */
			virtual std::string getString() const = 0;

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

			friend class ASTContext;

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

			PrimitiveType(Kind kd);

			virtual std::string getString() const override;

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

			virtual std::string getString() const override;

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

			virtual std::string getString() const override;

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

	// SemaType
	//		TypeBase used in semantic analysis to perform
	//		basic, local type inference. This is, like LValueType & ArrayType,
	//		just a wrapper around a TypeBase*, however the pointer may be null if no
	//		substitution was chosen.
	class SemaType : public TypeBase
	{
		public:
			static SemaType* create(ASTContext& ctxt, TypeBase* subst = nullptr);

			virtual std::string getString() const override;

			TypeBase* getSubstitution();
			const TypeBase* getSubstitution() const;

			// Returns true if the type has a substitution
			// (type isn't null)
			bool hasSubstitution() const;

			void setSubstitution(TypeBase* subst);

			// Sets the type to nullptr
			void reset();

			static bool classof(const TypeBase* type)
			{
				return (type->getKind() == TypeKind::SemaType);
			}

		private:
			// Private because only called by ::create
			SemaType(TypeBase* subst = nullptr);

			TypeBase * ty_ = nullptr;
	};

	// ErrorType
	//		A type used to represent that a expression's type
	//		cannot be determined because of an error.
	class ErrorType : public TypeBase
	{
		public:
			ErrorType();

			virtual std::string getString() const override;

			static bool classof(const TypeBase* type)
			{
				return (type->getKind() == TypeKind::ErrorType);
			}
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
		using CSList = std::list<Constraint*>;
		public:
			// Creates a new instance of the ConstrainedType class
			static ConstrainedType* create(ASTContext& ctxt);

			virtual std::string getString() const override;

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

			CSList::iterator cs_begin();
			CSList::const_iterator cs_begin() const;

			CSList::iterator cs_end();
			CSList::const_iterator cs_end() const;

			CSList& getConstraints();

			std::size_t numConstraints() const;
			void addConstraint(Constraint* cs);

			static bool classof(const TypeBase* type)
			{
				return (type->getKind() == TypeKind::ConstrainedType);
			}

		private:
			// Private because only called by ::create
			ConstrainedType();

			// Override the new/deletes to use the ConstraintAllocator in the
			// ASTContext to allocate ConstrainedTypes.

			// Only allow allocation through the ASTContext
			// This operator is "protected" so only the ASTContext can create types.
			void* operator new(std::size_t sz, ASTContext &ctxt, std::uint8_t align = alignof(ConstrainedType));

			// Companion operator delete to silence C4291 on MSVC
			void operator delete(void*, ASTContext&, std::uint8_t) {}

			void markAsUpToDate();
			void markAsOutdated();

			CSList constraints_;
			// Pointer + "up to date" marker. If the int is 1,
			// the pointer is up to date, 0 if it isnt.
			llvm::PointerIntPair<TypeBase*, 1> subst_;
	};

}