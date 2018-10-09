////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Constraint.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file contains the Constraint hierarchy, which represents
// a constraint imposed on a Type. This is used for the ConstraintedType.
// Constraint are immutable once created.
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

			// Returns this constraint's name (in a developer-friendly way, for debugging purposes)
			std::string toDebugString() const;

		protected:

			// Prohibit the use of builtin (placement) new & delete
			void *operator new(std::size_t) throw() = delete;
			void operator delete(void *) throw() = delete;
			void* operator new(std::size_t, void*) = delete;

			// Only allow allocation through the ASTContext
			void* operator new(std::size_t sz, ASTContext &ctxt, std::uint8_t align = alignof(Constraint));

			// Companion operator delete to silence C4291 on MSVC
			void operator delete(void*, ASTContext&, std::uint8_t) {}

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
			static EqualityCS* create(ASTContext& ctxt, Type& type);

			Type& getType();
			const Type& getType() const;

			static bool classof(const Constraint* cs)
			{
				return (cs->getKind() == Kind::EqualityCS);
			}
		
		private:
			EqualityCS(Type& type);

			Type& type_;
	};

	// ArrayCS
	//		Satisfied if the substitution is 
	//		an ArrayType.
	class ArrayCS : public Constraint
	{
		public:
			static ArrayCS* create(ASTContext& ctxt);

			static bool classof(const Constraint* cs)
			{
				return (cs->getKind() == Kind::ArrayCS);
			}

		private:
			ArrayCS();
	};
}