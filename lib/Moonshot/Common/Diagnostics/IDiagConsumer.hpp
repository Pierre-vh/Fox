////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : IDiagConsumer.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file declares the IDiagConsumer interface, which is used
// to consume diagnostics. This is a pretty basic class which only
// requires to define a consume method which...consumes a diagnostic.
////------------------------------------------------------////

#pragma once

namespace Moonshot
{
	class Diagnostic;
	class IDiagConsumer
	{
		public:
			virtual ~IDiagConsumer() = 0 { }
			virtual void consume(const Diagnostic& diag) = 0;
	};
}