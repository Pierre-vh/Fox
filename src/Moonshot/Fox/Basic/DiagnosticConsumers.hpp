////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : DiagnosticConsumers.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file contains the DiagnosticConsumer interface as well
// as some builtin implementations.
////------------------------------------------------------////

#pragma once

#include <string>
#include <iostream>

namespace Moonshot
{
	class Diagnostic;
	enum class DiagSeverity : int8_t;
	class DiagnosticConsumer
	{
		public:
			virtual ~DiagnosticConsumer() = 0 { }
			virtual void consume(const Diagnostic& diag) = 0;
	};
	class StreamDiagConsumer : public DiagnosticConsumer
	{
		public:
			StreamDiagConsumer(std::ostream& stream = std::cout); // Default outstream is cout (stdio)
			virtual void consume(const Diagnostic& diag) override;
		private:
			std::string diagSevToString(const DiagSeverity& ds) const;

			std::ostream &os;
	};
}
