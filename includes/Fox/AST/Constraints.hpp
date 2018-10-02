////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Constraints.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file contains the Constraints hierarchy, which represents
// a constraint imposed on a Type. This is used for the ConstraintedType.
// Constraints are immutable once created.
//
//	TO-DO/Critics:
//		EqualityCS: Type, Type& or TypeBase*?
//		Is a hierarchy the correct way to do things? Some classes are empty!
//			Hierarchy offers more flexibility to add "locators" or stuff like that
//			later, if Semantic analysis needs it.
//
////------------------------------------------------------////

#pragma once

#include <cstdint>
#include <cstddef>
#include "Type.hpp"

namespace fox
{
	class ASTContext;

	// Base class for every constraint
	class Constraint
	{
		public:
			enum class Kind : std::uint8_t
			{
				#define CS(ID, PARENT) ID,
				#include "Constraints.def"
			};

			Kind getKind() const;

			// Prohibit the use of builtin (placement) new & delete
			void *operator new(std::size_t) throw() = delete;
			void operator delete(void *) throw() = delete;
			void* operator new(std::size_t, void*) = delete;

			// Only allow allocation through the ASTContext
			void* operator new(std::size_t sz, ASTContext &ctxt, std::uint8_t align = alignof(Constraint));

			// Companion operator delete to silence C4291 on MSVC
			void operator delete(void*, ASTContext&, std::uint8_t) {}

		protected:
			Constraint(Kind kind);

		private:
			const Kind kind_;

	};
	
	// EqualityCS
	//		Satisfied if the substitution matches
	//		the type contained in this constraint.
	class EqualityCS : public Constraint
	{
		public:
			EqualityCS(Type& type);

			Type& getType();
			const Type& getType() const;

			static bool classof(const Constraint* cs)
			{
				return (cs->getKind() == Kind::EqualityCS);
			}
		private:
			Type& type_;
	};

	// ArrayCS
	//		Satisfied if the substitution is 
	//		an ArrayType.
	class ArrayCS : public Constraint
	{
		public:
			ArrayCS();

			static bool classof(const Constraint* cs)
			{
				return (cs->getKind() == Kind::ArrayCS);
			}
	};
}