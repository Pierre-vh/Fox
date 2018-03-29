////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : BuiltinDiagConsumers.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file declares diag consumers that are used by the
// base project, mainly, the StreamDiagConsumer.
////------------------------------------------------------////

#pragma once

#include "IDiagConsumer.hpp"

#include "Diagnostic.hpp"
#include <string>
#include <iostream>

namespace Moonshot
{
	class StreamDiagConsumer : public IDiagConsumer
	{
		public:
			StreamDiagConsumer(std::ostream& stream = std::cout); // Default outstream is cout (stdio)
			virtual void consume(const Diagnostic& diag) override;
		private:
			std::string diagSevToString(const DiagSeverity& ds) const;

			std::ostream &os;
	};
}
