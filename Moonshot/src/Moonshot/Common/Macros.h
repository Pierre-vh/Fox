////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Macros.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Declares useful macros.										
////------------------------------------------------------////

#pragma once
// This file defines various useful GENERAL macros.
// Other macros are defined in their corresponding files, like in Error.h

// Macro from
// https://stackoverflow.com/a/2175441/3232822
// MUST BE PRIVATE!
#define DISALLOW_COPY_AND_ASSIGN(TypeName)\
  TypeName(const TypeName&) = delete;      \
  void operator=(const TypeName&) = delete;