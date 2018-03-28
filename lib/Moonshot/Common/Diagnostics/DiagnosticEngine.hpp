////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : DiagnosticEngine.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file declares the DiagnosticEngine class, which is used
// to coordinate diagnostic generation and consumption.
////------------------------------------------------------////

#pragma once

#include "IDiagConsumer.hpp"
#include "Moonshot/Common/Utils/Utils.hpp"

#include <memory>

namespace Moonshot
{
	class Diagnostic;
	enum class DiagsID;
	class DiagnosticEngine
	{
		public:
			DiagnosticEngine();
			DiagnosticEngine(std::unique_ptr<IDiagConsumer> ncons);

			Diagnostic report(const DiagsID& diagID);

			void setConsumer(std::unique_ptr<IDiagConsumer> ncons);
			IDiagConsumer * getConsumer();
		private:
			std::unique_ptr<IDiagConsumer> consumer_ = 0;
	};
}