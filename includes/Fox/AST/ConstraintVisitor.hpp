////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ConstraintVisitor.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file contains the ConstraintVisitor. It has
// an interface similar to the ASTVisitor, except that it's
// reserved to constraints.
// This is implemented here and not in the ASTVisitor because
// constraints aren't really a part of the AST.
////------------------------------------------------------//// 

#pragma once

#include "Fox/Common/Errors.hpp"
#include "Constraints.hpp"
#include <utility>

namespace fox
{
	template<typename Derived, typename RtrTy, typename ... Args>
	class ConstraintVisitor
	{
		public:
			// Entry point for visiting a Constraint
			RtrTy visit(Constraint* cs, Args... args)
			{
				assert(cs && "Cannot be used on a null pointer");
				switch (cs->getKind())
				{
					#define CS(ID, PARENT)\
							case Constraint::Kind::ID:\
								return static_cast<Derived*>(this)->visit##ID(cs, ::std::forward<Args>(args)...);
					#include "Constraints.def"
					default:
						fox_unreachable("Unknown constraint");
				}
			}

			// Visit methods for constraints.
			#define CS(ID, PARENT)\
			RtrTy visit##ID(Constraint*, Args...){ \
				return RtrTy(); \
			}
			#include "Constraints.def"
	};
}