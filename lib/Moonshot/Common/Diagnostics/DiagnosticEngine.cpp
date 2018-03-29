////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : DiagnosticEngine.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "DiagnosticEngine.hpp"
#include "Diagnostic.hpp"
#include "BuiltinDiagConsumers.hpp"

using namespace Moonshot;

static const char* diagsStrs[] = {
	#define DIAG(SEVERITY,ID,TEXT) TEXT,
	#define KEEP_DIAG_DEF
		#include "DiagsAll.def"
	#undef DIAG
	#undef KEEP_DIAG_DEF
};

static const DiagSeverity diagsSevs[] = {
	#define DIAG(SEVERITY,ID,TEXT) DiagSeverity::SEVERITY,
	#define KEEP_DIAG_DEF
		#include "DiagsAll.def"
	#undef DIAG
	#undef KEEP_DIAG_DEF
};

DiagnosticEngine::DiagnosticEngine()
{
	consumer_ = std::make_unique<StreamDiagConsumer>(); // Default diag consumer outputs to cout
}

DiagnosticEngine::DiagnosticEngine(std::unique_ptr<IDiagConsumer> ncons): consumer_(std::move(ncons))
{

}

Diagnostic DiagnosticEngine::report(const DiagID & diagID)
{
	const auto idx = Util::enumAsInt(diagID);
	return Diagnostic(
			consumer_.get(),
			diagID,
			diagsSevs[idx],
			std::string(diagsStrs[idx])
		);
}

void DiagnosticEngine::setConsumer(std::unique_ptr<IDiagConsumer> ncons)
{
	consumer_ = std::move(ncons);
}

IDiagConsumer* DiagnosticEngine::getConsumer()
{
	return consumer_.get();
}
