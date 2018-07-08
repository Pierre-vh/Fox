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

#include <iostream>

namespace fox
{
	class Diagnostic;
	class SourceLoc;
	class SourceRange;
	class SourceManager;
	enum class DiagSeverity : std::uint8_t;

	class DiagnosticConsumer
	{
		public:
			virtual void consume(const Diagnostic& diag) = 0;
	};

	class StreamDiagConsumer : public DiagnosticConsumer
	{
		public:
			StreamDiagConsumer(SourceManager* sm,std::ostream& stream = std::cout); // Default outstream is cout (stdio)
			virtual void consume(const Diagnostic& diag) override;

		private:
			std::string getLocInfo(const SourceRange& range, bool isFileWide) const;
			std::string diagSevToString(DiagSeverity ds) const;

			SourceManager* sm_;
			std::ostream &os_;
	};
}
