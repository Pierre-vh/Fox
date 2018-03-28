////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : BuiltinDiagConsumers.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file declares several diag consumers that are used by the
// base project.
////------------------------------------------------------////

#pragma once

#include "IDiagConsumer.hpp"

namespace Moonshot
{
	class StdIoDiagConsumer : public IDiagConsumer
	{
		virtual void consume(const Diagnostic& diag) override;
	};
}
