////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : TypesUtils.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Utility functions for the FoxType type and things revolving around it.
////------------------------------------------------------////

#pragma once

#include "Types.hpp"

namespace Moonshot::TypeUtils
{


	// Checks if assignement is possible.
	bool canAssign(const FoxType &lhs, const FoxType &rhs); // Checks if the lhs and rhs are compatible.
																	// Compatibility : 
																	// Arithmetic type <-> Arithmetic Type = ok
																	// same type <-> same type = ok
																	// else : error.

																	// This function returns true if the type of basetype can be cast to the type of goal.
	bool canImplicitelyCastTo(const FoxType &goal, const FoxType &basetype);
	bool canExplicitelyCastTo(const FoxType &goal, const FoxType &basetype);
	bool canConcat(const FoxType& lhs, const FoxType& rhs);
	FoxType getBiggestType(const FoxType &lhs, const FoxType &rhs); // returns the largest (biggest) type of the 2 arguments.
}